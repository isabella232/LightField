#include "dlpc350impl.h"
#include <thread>

unsigned int ProjectorDlpc350Impl::getLEDBrightness()
{
    unsigned int brightness = 0;

    return brightness;
}

bool ProjectorDlpc350Impl::openPort()
{
    hid_init();

    // Open the device using the VID, PID,
    // and optionally the Serial number.
    deviceHandle = hid_open(MY_VID, MY_PID, NULL);

    if (deviceHandle == NULL)
    {
        USBConnected = 0;
        return false;
    }

    USBConnected = 1;

    return true;
}

bool ProjectorDlpc350Impl::closePort()
{
    hid_close(deviceHandle);
    USBConnected = 0;

    return true;
}

bool ProjectorDlpc350Impl::firstTimeConfiguration()
{
    return true;
}

bool ProjectorDlpc350Impl::setPowerLevel(unsigned long powerLevel)
{
    if (DLPC350_SetLedEnables(false, false, false, true) < 0) //set blue channel power level
        return false;

    if (DLPC350_SetLedCurrents(0, 0, 255 - powerLevel) < 0)
    {
        return false;
    }

    return true;
}

bool ProjectorDlpc350Impl::setDuration(int duration)
{
    std::thread t([this, duration]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        setPowerLevel(0);
    });

    t.join();
    return false;
}

unsigned long ProjectorDlpc350Impl::getPowerLevel()
{
    unsigned char r, g, b;
    DLPC350_GetLedCurrents(&r, &g, &b);

    return 255 - b;
}

unsigned int ProjectorDlpc350Impl::getLEDTemperature()
{
    unsigned int temperature = 0;

    return temperature;
};

int ProjectorDlpc350Impl::DLPC350_USB_Write()
{
    int bytesWritten;

    if (deviceHandle == NULL)
        return -1;

    if ((bytesWritten = hid_write(deviceHandle, gOutputBuffer, USB_MIN_PACKET_SIZE + 1)) == -1)
    {
        closePort();
        return -1;
    }

    return bytesWritten;
}

int ProjectorDlpc350Impl::DLPC350_USB_Read()
{
    int bytesRead;

    if (deviceHandle == NULL)
        return -1;

    //clear out the input buffer
    memset((void *)&gInputBuffer[0], 0x00, USB_MIN_PACKET_SIZE + 1);

    if ((bytesRead = hid_read_timeout(deviceHandle, gInputBuffer, USB_MIN_PACKET_SIZE + 1, 2000)) == -1)
    {
        hid_close(deviceHandle);
        USBConnected = 0;
        return -1;
    }

    return bytesRead;
}

int ProjectorDlpc350Impl::DLPC350_Write(bool ackRequired)
{
    int ret_val;
    hidMessageStruct *pMsg;

    if (ackRequired)
    {
        pMsg = (hidMessageStruct *)gInputBuffer;
        if ((ret_val = DLPC350_USB_Write()) > 0)
        {
            //Check for ACK or NACK response
            if (DLPC350_USB_Read() > 0)
            {
                if (pMsg->head.flags.nack == 1)
                    return -2;
                else
                    return ret_val;
            }
        }
    }
    else
    {
        ret_val = DLPC350_USB_Write();
    }

    return ret_val;
}

int ProjectorDlpc350Impl::DLPC350_SendMsg(hidMessageStruct *pMsg, bool ackRequired)
{
    int maxDataSize = USB_MAX_PACKET_SIZE - sizeof(pMsg->head);
    int dataBytesSent = MIN(pMsg->head.length, maxDataSize); //Send all data or max possible

    // Default the DLPC350_PrepWriteCmd() update write message for ACK
    // if user not expecting adjust accordingly
    if (!ackRequired)
        pMsg->head.flags.reply = 0;

    gOutputBuffer[0] = 0; // First byte is the report number
    memcpy(&gOutputBuffer[1], pMsg, (sizeof(pMsg->head) + dataBytesSent));

    //Single packet transaction
    if (dataBytesSent >= pMsg->head.length)
    {
        if (DLPC350_Write(ackRequired) < 0)
            return -1;
    }
    else
    {
        //Send ACK request only for the last packet
        if (DLPC350_Write(0) < 0)
            return -1;

        while (dataBytesSent < pMsg->head.length)
        {
            memcpy(&gOutputBuffer[1], &pMsg->text.data[dataBytesSent], USB_MAX_PACKET_SIZE);

            if ((dataBytesSent + USB_MAX_PACKET_SIZE) >= (pMsg->head.length))
            {
                //last packet request for ACK
                if (DLPC350_Write(ackRequired) < 0)
                    return -1;
            }
            else
            {
                //middle packet
                if (DLPC350_Write(0) < 0)
                    return -1;
            }

            dataBytesSent += USB_MAX_PACKET_SIZE;
        }
    }

    return dataBytesSent + sizeof(pMsg->head);
}

int ProjectorDlpc350Impl::DLPC350_SetLedEnables(bool SeqCtrl, bool Red, bool Green, bool Blue)
{
    hidMessageStruct msg;
    unsigned char Enable = 0;

    if (SeqCtrl)
        Enable |= BIT3;
    if (Red)
        Enable |= BIT0;
    if (Green)
        Enable |= BIT1;
    if (Blue)
        Enable |= BIT2;

    msg.text.data[2] = Enable;
    DLPC350_PrepWriteCmd(&msg, LED_ENABLE);

    return DLPC350_SendMsg(&msg, true);
}

int ProjectorDlpc350Impl::DLPC350_PrepWriteCmd(hidMessageStruct *pMsg, DLPC350_CMD cmd)
{
    pMsg->head.flags.rw = 0;    //Write
    pMsg->head.flags.reply = 1; //Host wants a reply from device
    pMsg->head.flags.dest = 0;  //Projector Control Endpoint
    pMsg->head.flags.reserved = 0;
    pMsg->head.flags.nack = 0;
    pMsg->head.seq = gSeqNum++;

    pMsg->text.cmd = (CmdList[cmd].CMD2 << 8) | CmdList[cmd].CMD3;
    pMsg->head.length = CmdList[cmd].len + 2;

    return 0;
}

int ProjectorDlpc350Impl::DLPC350_SetLedCurrents(unsigned char RedCurrent, unsigned char GreenCurrent, unsigned char BlueCurrent)
{
    hidMessageStruct msg;

    msg.text.data[2] = RedCurrent;
    msg.text.data[3] = GreenCurrent;
    msg.text.data[4] = BlueCurrent;

    DLPC350_PrepWriteCmd(&msg, LED_CURRENT);

    return DLPC350_SendMsg(&msg, true);
}

int ProjectorDlpc350Impl::DLPC350_PrepReadCmd(DLPC350_CMD cmd)
{
    hidMessageStruct msg;

    msg.head.flags.rw = 1;    //Read
    msg.head.flags.reply = 1; //Host wants a reply from device
    msg.head.flags.dest = 0;  //Projector Control Endpoint
    msg.head.flags.reserved = 0;
    msg.head.flags.nack = 0;
    msg.head.seq = 0;

    msg.text.cmd = (CmdList[cmd].CMD2 << 8) | CmdList[cmd].CMD3;
    msg.head.length = 2;

    if (cmd == BL_GET_MANID)
    {
        msg.text.data[2] = 0x0C;
        msg.head.length += 1;
    }
    else if (cmd == BL_GET_DEVID)
    {
        msg.text.data[2] = 0x0D;
        msg.head.length += 1;
    }
    else if (cmd == BL_GET_CHKSUM)
    {
        msg.text.data[2] = 0x00;
        msg.head.length += 1;
    }

    gOutputBuffer[0] = 0; // First byte is the report number
    memcpy(&gOutputBuffer[1], &msg, (sizeof(msg.head) + sizeof(msg.text.cmd) + msg.head.length));
    return 0;
}

int ProjectorDlpc350Impl::DLPC350_Read()
{
    int ret_val;
    hidMessageStruct *pMsg = (hidMessageStruct *)gInputBuffer;
    if (DLPC350_USB_Write() > 0)
    {
        ret_val = DLPC350_USB_Read();

        if ((pMsg->head.flags.nack == 1) || (pMsg->head.length == 0))
            return -2;
        else
            return ret_val;
    }
    return -1;
}

int ProjectorDlpc350Impl::DLPC350_GetLedCurrents(unsigned char *pRed, unsigned char *pGreen, unsigned char *pBlue)
{
    hidMessageStruct msg;

    DLPC350_PrepReadCmd(LED_CURRENT);
    if (DLPC350_Read() > 0)
    {
        memcpy(&msg, gInputBuffer, 65);

        *pRed = msg.text.data[0];
        *pGreen = msg.text.data[1];
        *pBlue = msg.text.data[2];

        return 0;
    }
    return -1;
}