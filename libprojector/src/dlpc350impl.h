#ifndef PROJECTORDLPC350IMPL
#define PROJECTORDLPC350IMPL

#include <hidapi/hidapi.h>
#include <cstring>

#include "libprojector.h"

#define USB_MIN_PACKET_SIZE 64
#define USB_MAX_PACKET_SIZE 64

#define MY_VID 0x0451
#define MY_PID 0x6401

#define HID_MESSAGE_MAX_SIZE 512

/* Bit masks. */
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80
#define BIT8 0x0100
#define BIT9 0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef struct _readCmdData
{
    unsigned char CMD2;
    unsigned char CMD3;
    unsigned short len;
} CmdFormat;

typedef struct _hidmessageStruct
{
    struct _hidhead
    {
        struct _packetcontrolStruct
        {
            unsigned char dest : 3; /* 0 - ProjCtrl; 1 - RFC; 7 - Debugmsg */
            unsigned char reserved : 2;
            unsigned char nack : 1;  /* Command Handler Error */
            unsigned char reply : 1; /* Host wants a reply from device */
            unsigned char rw : 1;    /* Write = 0; Read = 1 */
        } flags;
        unsigned char seq;
        unsigned short length;
    } head;
    union
    {
        unsigned short cmd;
        unsigned char data[HID_MESSAGE_MAX_SIZE];
    } text;
} hidMessageStruct;

typedef enum
{
    VID_SIG_STAT,
    SOURCE_SEL,
    PIXEL_FORMAT,
    CLK_SEL,
    CHANNEL_SWAP,
    FPD_MODE,
    CURTAIN_COLOR,
    POWER_CONTROL,
    FLIP_LONG,
    FLIP_SHORT,
    TPG_SEL,
    PWM_INVERT,
    LED_ENABLE,
    GET_VERSION,
    GET_FIRMWAE_TAG_INFO,
    SW_RESET,
    DMD_PARK,
    BUFFER_FREEZE,
    STATUS_HW,
    STATUS_SYS,
    STATUS_MAIN,
    CSC_DATA,
    GAMMA_CTL,
    BC_CTL,
    PWM_ENABLE,
    PWM_SETUP,
    PWM_CAPTURE_CONFIG,
    GPIO_CONFIG,
    LED_CURRENT,
    DISP_CONFIG,
    TEMP_CONFIG,
    TEMP_READ,
    MEM_CONTROL,
    I2C_CONTROL,
    LUT_VALID,
    DISP_MODE,
    TRIG_OUT1_CTL,
    TRIG_OUT2_CTL,
    RED_STROBE_DLY,
    GRN_STROBE_DLY,
    BLU_STROBE_DLY,
    PAT_DISP_MODE,
    PAT_TRIG_MODE,
    PAT_START_STOP,
    BUFFER_SWAP,
    BUFFER_WR_DISABLE,
    CURRENT_RD_BUFFER,
    PAT_EXPO_PRD,
    INVERT_DATA,
    PAT_CONFIG,
    MBOX_ADDRESS,
    MBOX_CONTROL,
    MBOX_DATA,
    TRIG_IN1_DELAY,
    TRIG_IN2_CONTROL,
    IMAGE_LOAD,
    IMAGE_LOAD_TIMING,
    I2C0_CTRL,
    MBOX_EXP_DATA,
    MBOX_EXP_ADDRESS,
    EXP_PAT_CONFIG,
    NUM_IMAGE_IN_FLASH,
    I2C0_STAT,
    GPCLK_CONFIG,
    PULSE_GPIO_23,
    ENABLE_DLPC350_DEBUG,
    TPG_COLOR,
    PWM_CAPTURE_READ,
    PROG_MODE,
    BL_STATUS,
    BL_SPL_MODE,
    BL_GET_MANID,
    BL_GET_DEVID,
    BL_GET_CHKSUM,
    BL_SET_SECTADDR,
    BL_SECT_ERASE,
    BL_SET_DNLDSIZE,
    BL_DNLD_DATA,
    BL_FLASH_TYPE,
    BL_CALC_CHKSUM,
    BL_PROG_MODE
} DLPC350_CMD;

class ProjectorDlpc350Impl : public ProjectorController
{
public:
    unsigned int getLEDBrightness() override;
    bool openPort() override;
    bool closePort() override;
    bool firstTimeConfiguration() override;
    bool setPowerLevel(unsigned long powerLevel) override;
    bool setDuration(int duration) override;
    unsigned long getPowerLevel() override;
    unsigned int getLEDTemperature() override;
    static bool devicePlugged() {
        hid_device_info* list = hid_enumerate(MY_VID, MY_PID);
        bool result = list != NULL;

        if(result)
            hid_free_enumeration(list);

        return result;
    }
protected:
    int DLPC350_USB_Write();
    int DLPC350_USB_Read();

private:
    hid_device *deviceHandle; //Handle to write
    int USBConnected{0};
    unsigned char gOutputBuffer[USB_MAX_PACKET_SIZE + 1];
    unsigned char gInputBuffer[USB_MAX_PACKET_SIZE + 1];
    unsigned char gSeqNum = 0;

    int DLPC350_Write(bool ackRequired);

    /**
     * This function is private to this file. This function is called to send a message over USB; in chunks of 64 bytes.
     *
     * @return  number of bytes sent
     *          -1 = FAIL
     *
     */
    int DLPC350_SendMsg(hidMessageStruct *pMsg, bool ackRequired);

    /**
     * This command sets the state of LED control method as well as the enabled/disabled status of all LEDs.
     * (I2C: 0x10)
     * (USB: CMD2: 0x1A, CMD3: 0x07)
     *
     * @param   pSeqCtrl  - I - 1 - All LED enables are controlled by the Sequencer and ignore the other LED enable settings.
     *                          0 - All LED enables are controlled by pRed, pGreen and pBlue seetings and ignore Sequencer control
     * @param   pRed  - I - 0 - Red LED is disabled
     *                      1 - Red LED is enabled
     * @param   pGreen  - I - 0 - Green LED is disabled
     *                      1 - Green LED is enabled
     * @param   pBlue  - I - 0 - Blue LED is disabled
     *                      1 - Blue LED is enabled]
     *
     * @return  0 = PASS    <BR>
     *          -1 = FAIL  <BR>
     *
     */
    int DLPC350_SetLedEnables(bool SeqCtrl, bool Red, bool Green, bool Blue);

    /**
     * This function is private to this file. Prepares the write command packet with given command code in the message structure pointer passed.
     *
     * @param   cmd  - I - USB command code.
     * @param   pMsg - I - Pointer to the message.
     *
     * @return  0 = PASS
     *          -1 = FAIL
     *
     */
    int DLPC350_PrepWriteCmd(hidMessageStruct *pMsg, DLPC350_CMD cmd);

    /**
     * (I2C: 0x4B)
     * (USB: CMD2: 0x0B, CMD3: 0x01)
     * This parameter controls the pulse duration of the specific LED PWM modulation output pin. The resolution
     * is 8 bits and corresponds to a percentage of the LED current. The PWM value can be set from 0 to 100%
     * in 256 steps . If the LED PWM polarity is set to normal polarity, a setting of 0xFF gives the maximum
     * PWM current. The LED current is a function of the specific LED driver design.
     *
     * @param   RedCurrent  - I - Red LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
     *                      0x00 (0% duty cycle → Red LED driver generates no current
     *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
     *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
     * @param   GreenCurrent  - I - Green LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
     *                      0x00 (0% duty cycle → Red LED driver generates no current
     *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
     *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
     * @param   BlueCurrent  - I - Blue LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
     *                      0x00 (0% duty cycle → Red LED driver generates no current
     *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
     *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
     *
     * @return  0 = PASS    <BR>
     *          -1 = FAIL  <BR>
     *
     */
    int DLPC350_SetLedCurrents(unsigned char RedCurrent, unsigned char GreenCurrent, unsigned char BlueCurrent);

    /**
     * This function is private to this file. Prepares the read-control command packet for the given command code and copies it to g_OutputBuffer.
     *
     * @param   cmd  - I - USB command code.
     *
     * @return  0 = PASS
     *          -1 = FAIL
     *
     */
    int DLPC350_PrepReadCmd(DLPC350_CMD cmd);

    /**
     * This function is private to this file. This function is called to write the read control command and then read back 64 bytes over USB
     * to g_InputBuffer.
     *
     * @return  number of bytes read
     *          -2 = nack from target
     *          -1 = error reading
     *
     */
    int DLPC350_Read();

    /**
     * (I2C: 0x4B)
     * (USB: CMD2: 0x0B, CMD3: 0x01)
     * This parameter controls the pulse duration of the specific LED PWM modulation output pin. The resolution
     * is 8 bits and corresponds to a percentage of the LED current. The PWM value can be set from 0 to 100%
     * in 256 steps . If the LED PWM polarity is set to normal polarity, a setting of 0xFF gives the maximum
     * PWM current. The LED current is a function of the specific LED driver design.
     *
     * @param   pRed  - O - Red LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
     *                      0x00 (0% duty cycle → Red LED driver generates no current
     *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
     *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
     * @param   pGreen  - O - Green LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
     *                      0x00 (0% duty cycle → Red LED driver generates no current
     *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
     *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
     * @param   pBlue  - O - Blue LED PWM current control Valid range, assuming normal polarity of PWM signals, is:
     *                      0x00 (0% duty cycle → Red LED driver generates no current
     *                      0xFF (100% duty cycle → Red LED driver generates maximum current))
     *                      The current level corresponding to the selected PWM duty cycle is a function of the specific LED driver design and thus varies by design.
     *
     * @return  0 = PASS    <BR>
     *          -1 = FAIL  <BR>
     *
     */
    int DLPC350_GetLedCurrents(unsigned char *pRed, unsigned char *pGreen, unsigned char *pBlue);

    CmdFormat CmdList[255] =
        {
            {0x07, 0x1C, 0x1C}, //VID_SIG_STAT,
            {0x1A, 0x00, 0x01}, //SOURCE_SEL,
            {0x1A, 0x02, 0x01}, //PIXEL_FORMAT,
            {0x1A, 0x03, 0x01}, //CLK_SEL,
            {0x1A, 0x37, 0x01}, //CHANNEL_SWAP,
            {0x1A, 0x04, 0x01}, //FPD_MODE,
            {0, 0, 0},          //CURTAIN_COLOR,
            {0x02, 0x00, 0x01}, //POWER_CONTROL,
            {0x10, 0x08, 0x01}, //FLIP_LONG,
            {0x10, 0x09, 0x01}, //FLIP_SHORT,
            {0x12, 0x03, 0x01}, //TPG_SEL,
            {0x1A, 0x05, 0x01}, //PWM_INVERT,
            {0x1A, 0x07, 0x01}, //LED_ENABLE,
            {0x02, 0x05, 0x00}, //GET_VERSION,
            {0x1A, 0xFF, 0x00}, //GET_FIRMWAE_TAG_INFO
            {0x08, 0x02, 0x00}, //SW_RESET,
            {0, 0, 0},          //DMD_PARK,
            {0x10, 0x0A, 0x01}, //BUFFER_FREEZE,
            {0x1A, 0x0A, 0x00}, //STATUS_HW,
            {0x1A, 0x0B, 0x00}, //STATUS_SYS,
            {0x1A, 0x0C, 0x00}, //STATUS_MAIN,
            {0, 0, 0},          //CSC_DATA,
            {0, 0, 0},          //GAMMA_CTL,
            {0, 0, 0},          //BC_CTL,
            {0x1A, 0x10, 0x01}, //PWM_ENABLE,
            {0x1A, 0x11, 0x06}, //PWM_SETUP,
            {0x1A, 0x12, 0x05}, //PWM_CAPTURE_CONFIG,
            {0x1A, 0x38, 0x02}, //GPIO_CONFIG,
            {0x0B, 0x01, 0x03}, //LED_CURRENT,
            {0x10, 0x00, 0x10}, //DISP_CONFIG,
            {0, 0, 0},          //TEMP_CONFIG,
            {0, 0, 0},          //TEMP_READ,
            {0x1A, 0x16, 0x09}, //MEM_CONTROL,
            {0, 0, 0},          //I2C_CONTROL,
            {0x1A, 0x1A, 0x01}, //LUT_VALID,
            {0x1A, 0x1B, 0x01}, //DISP_MODE,
            {0x1A, 0x1D, 0x03}, //TRIG_OUT1_CTL,
            {0x1A, 0x1E, 0x02}, //TRIG_OUT2_CTL,
            {0x1A, 0x1F, 0x02}, //RED_STROBE_DLY,
            {0x1A, 0x20, 0x02}, //GRN_STROBE_DLY,
            {0x1A, 0x21, 0x02}, //BLU_STROBE_DLY,
            {0x1A, 0x22, 0x01}, //PAT_DISP_MODE,
            {0x1A, 0x23, 0x01}, //PAT_TRIG_MODE,
            {0x1A, 0x24, 0x01}, //PAT_START_STOP,
            {0, 0, 0},          //BUFFER_SWAP,
            {0, 0, 0},          //BUFFER_WR_DISABLE,
            {0, 0, 0},          //CURRENT_RD_BUFFER,
            {0x1A, 0x29, 0x08}, //PAT_EXPO_PRD,
            {0x1A, 0x30, 0x01}, //INVERT_DATA,
            {0x1A, 0x31, 0x04}, //PAT_CONFIG,
            {0x1A, 0x32, 0x01}, //MBOX_ADDRESS,
            {0x1A, 0x33, 0x01}, //MBOX_CONTROL,
            {0x1A, 0x34, 0x00}, //MBOX_DATA,
            {0x1A, 0x35, 0x04}, //TRIG_IN1_DELAY,
            {0x1A, 0x36, 0x01}, //TRIG_IN2_CONTROL,
            {0x1A, 0x39, 0x01}, //IMAGE_LOAD,
            {0x1A, 0x3A, 0x02}, //IMAGE_LOAD_TIMING,
            {0x1A, 0x3B, 0x00}, //I2C0_CTRL,
            {0x1A, 0x3E, 0x0C}, //MBOX_EXP_DATA,
            {0x1A, 0x3F, 0x02}, //MBOX_EXP_ADDRESS,
            {0x1A, 0x40, 0x06}, //EXP_PAT_CONFIG
            {0x1A, 0x42, 0x01}, //NUM_IMAGE_IN_FLASH,
            {0x1A, 0x43, 0x01}, //I2C0_STAT,
            {0x08, 0x07, 0x03}, //GPCLK_CONFIG,
            {0, 0, 0},          //PULSE_GPIO_23,
            {0, 0, 0},          //ENABLE_DLPC350_DEBUG,
            {0x12, 0x04, 0x0C}, //TPG_COLOR,
            {0x1A, 0x13, 0x05}, //PWM_CAPTURE_READ,
            {0x30, 0x01, 0x00}, //PROG_MODE,
            {0x00, 0x00, 0x00}, //BL_STATUS
            {0x00, 0x23, 0x01}, //BL_SPL_MODE
            {0x00, 0x15, 0x01}, //BL_GET_MANID,
            {0x00, 0x15, 0x01}, //BL_GET_DEVID,
            {0x00, 0x15, 0x01}, //BL_GET_CHKSUM,
            {0x00, 0x29, 0x04}, //BL_SETSECTADDR,
            {0x00, 0x28, 0x00}, //BL_SECT_ERASE,
            {0x00, 0x2C, 0x04}, //BL_SET_DNLDSIZE,
            {0x00, 0x25, 0x00}, //BL_DNLD_DATA,
            {0x00, 0x2F, 0x01}, //BL_FLASH_TYPE,
            {0x00, 0x26, 0x00}, //BL_CALC_CHKSUM,
            {0x00, 0x30, 0x01}  //BL_PROG_MODE,
        };
};

#endif //PROJECTORDLPC350IMPL