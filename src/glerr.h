#ifndef __GLERR_H__
#define __GLERR_H__

#define GET_GL_ERRORS do { GetGlErrors( __PRETTY_FUNCTION__, __LINE__ ); } while ( false )

void GetGlErrors( char const* func, int line );
void ClearGlErrors( );

#endif // __GLERR_H__
