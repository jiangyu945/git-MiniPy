#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "exifparam.h"

#define IN
#define OUT
#define UCHAR unsigned char
#define SATIR
#define EXIF_FILE_SIZE                          28800
#define MAX_JPG_THUMBNAIL_WIDTH         320
#define MAX_JPG_THUMBNAIL_HEIGHT        240
#define MAX_FILE_THUMB_SIZE                     (MAX_JPG_THUMBNAIL_WIDTH * MAX_JPG_THUMBNAIL_HEIGHT)

extern int ex_value;

static const int APP1HeaderLen = 0x0C;
static UCHAR APP1Marker[] = {0xFF, 0xE1};
static UCHAR SATTag[] = {'S', 'A', 'T', 'I', 'R', 0x0};
static UCHAR MAX_APP1_LEN[] = {0xFF, 0xFE};
static const int MAX_DATA_LEN = 0xFFF4;


UCHAR ExifHeader[6]=
{
        0x45,0x78,0x69,0x66,0x00,0x00
};

UCHAR TIFFHeader[8]=
{
        0x49,0x49,0x2A,0x00,0x08,0x00,0x00,0x00
};

typedef union BytesOfShort
{
        UCHAR b[2];
        unsigned short s;
} BytesOfShort_t;


enum EXTRACT_DATE_JOB_STATE{
        EXTRACT_START=0,
        READ_EXIF_HEADER=1,
        READ_CHECK_SAT_HEADER,
        COPY_REMAINING_DATA,
        EXTRACT_END
};


/*
 *  构造EXIF信息
 *  @param  ExifOut   EXIF信息缓冲区
 *  @param  totalLen  EXIF信息长度
 *  @param  exifFileInfo EXIF结构体
 *  @return int  返回值，0为成功，-1为出错
 */
static int makeExifFile ( char *ExifOut, UINT *totalLen ,ExifFileInfo *exifFileInfo)
{
    UCHAR *ExifInitialCount;
    UCHAR *tempExif = ( UCHAR * ) ExifOut;
    INT32 ExifSize;
    UINT santemp;
    UCHAR * startoftiff;
    UCHAR * IFD1OffsetAddress;
    UCHAR APP1Marker[2]=	{0xff,0xe1};
    UCHAR ExifLen[4]={0};
    UCHAR Nentries[2]={8,0};
    UCHAR SubIFDNentries[2]={19,0};  //子IFD中的DE（Directory Entries）数量
    UCHAR IFD1Nentries[2]={6,0};
    UCHAR EndOfEntry[4]={0};

    //VARIABLES FOR THE MAKE OF THE CAMERA
    UCHAR  maketag[2]={0xf,0x1};
    UCHAR  makeformat[2]={0x2,0x0};
    UCHAR  Ncomponent[4]={32,0x0,0x0,0x0};
    char  make[32];
    UCHAR makeoffchar[4];
    UCHAR * offset;

    //VARIABLES FOR THE MODEL OF THE CAMERA
    UCHAR  modeltag[2]={0x10,0x1};
    UCHAR  modelformat[2]={0x2,0x0};
    UCHAR  NcomponentModel[4]={32,0x0,0x0,0x0};
    char  model[32];
    UCHAR modeloffchar[4];

    //VARIABLES FOR THE ORIENTATION OF THE CAMERA
    UCHAR  orientationtag[2]={0x12,0x1};
    UCHAR  orientationformat[2]={0x3,0x0};
    UCHAR  NcomponentOrientation[4]={0x1,0x0,0x0,0x0};
    UINT  Orientation[1];
    UCHAR  Orient[4] = {0};


    //VARIABLES FOR THE JPEG PROCESS
    UCHAR  Processtag[2]={0x00,0x02};
    UCHAR  Processformat[2]={0x3,0x0};
    UCHAR  NcomponentProcess[4]={0x1,0x0,0x0,0x0};
    UINT  Process[1];
    UCHAR  Proc[4] = {0};

    //VARIABLES FOR THE X-RESOLUTION OF THE IMAGE
    UCHAR  XResolutiontag[2]={0x1A,0x1};
    UCHAR  XResolutionformat[2]={0x5,0x0};
    UCHAR  NcomponentXResolution[4]={0x1,0x0,0x0,0x0};
    UINT  XResolutionNum[1];//={0x00000048};
    UINT  XResolutionDen[1];//={0x00000001};

    UCHAR XResolutionoffchar[4];
    UCHAR XResolutionNumChar[4];
    UCHAR XResolutionDenChar[4];

    //VARIABLES FOR THE Y-RESOLUTION OF THE IMAGE
    UCHAR  YResolutiontag[2]={0x1B,0x1};
    UCHAR  YResolutionformat[2]={0x5,0x0};
    UCHAR  NcomponentYResolution[4]={0x1,0x0,0x0,0x0};
    UINT  YResolutionNum[1];//={0x00000048};
    UINT  YResolutionDen[1];//={0x00000001};

    UCHAR YResolutionoffchar[4];
    UCHAR YResolutionNumChar[4];
    UCHAR YResolutionDenChar[4];

    //VARIABLES FOR THE RESOLUTION UNIT OF THE CAMERA
    UCHAR  RUnittag[2]={0x28,0x1};
    UCHAR  RUnitformat[2]={0x3,0x0};
    UCHAR  NcomponentRUnit[4]={0x1,0x0,0x0,0x0};
    UINT  RUnit[1];
    UCHAR  RUnitChar[4] = {0};


    //VARIABLES FOR THE VERSION NO OF THE SOFTWARE
    UCHAR  Versiontag[2]={0x31,0x1};
    UCHAR  Versionformat[2]={0x2,0x0};
    UCHAR  NcomponentVersion[4]={32,0x0,0x0,0x0};
    char  Version[32];//="version 1.2";
    UCHAR Versionoffchar[4];

    //VARIABLES FOR THE DATE/TIME
    UCHAR  DateTimetag[2]={0x32,0x1};
    UCHAR  DateTimeformat[2]={0x2,0x0};
    UCHAR  NcomponentDateTime[4]={20,0,0,0};
    UCHAR  DateTime[32];//="2006:6:09 15:17:32";
    char  DateTimeClose[1]={0};
    UCHAR DateTimeoffchar[4];

    //VARIABLES FOR THE COPYRIGHT
    UCHAR  CopyRighttag[2]={0x98,0x82};
    UCHAR  CopyRightformat[2]={0x2,0x0};
    UCHAR  NcomponentCopyRight[4]={32,0x0,0x0,0x0};
    char  CopyRight[32];
    UCHAR CopyRightoffchar[4];

    //VARIABLES FOR THE OFFSET TO SUBIFD
    UCHAR  SubIFDOffsettag[2]={0x69,0x87};
    UCHAR  SubIFDOffsetformat[2]={0x4,0x0};
    UCHAR  NcomponentSubIFDOffset[4]={0x1,0x0,0x0,0x0};
    UCHAR  SubIFDOffsetChar[4] = {0};


    //VARIABLES FOR THE EXPOSURE TIME
    UCHAR  ExposureTimetag[2]={0x9A,0x82};
    UCHAR  ExposureTimeformat[2]={0x5,0x0};
    UCHAR  NcomponentExposureTime[4]={0x1,0x0,0x0,0x0};
    UINT  ExposureTimeNum[1];
    UINT  ExposureTimeDen[1];

    UCHAR ExposureTimeoffchar[4];
    UCHAR ExposureTimeNumChar[4];
    UCHAR ExposureTimeDenChar[4];


    //VARIABLES FOR THE FNUMBER
    UCHAR  FNumbertag[2]={0x9D,0x82};
    UCHAR  FNumberformat[2]={0x5,0x0};
    UCHAR  NcomponentFNumber[4]={0x1,0x0,0x0,0x0};
    UINT  FNumberNum[1];
    UINT  FNumberDen[1];

    UCHAR FNumberoffchar[4];
    UCHAR FNumberNumChar[4];
    UCHAR FNumberDenChar[4];

    //VARIABLES FOR THE EXPOSURE PROGRAM OF THE CAMERA
    UCHAR  ExposureProgramtag[2]={0x22,0x88};
    UCHAR  ExposureProgramformat[2]={0x3,0x0};
    UCHAR  NcomponentExposureProgram[4]={0x1,0x0,0x0,0x0};
    UINT  ExposureProgram[1];
    UCHAR  ExposureProgramChar[4] = {0};

    //VARIABLES FOR THE ORIGINAL/TIME
    UCHAR  DateTimeOriginaltag[2]={0x03,0x90};
    UCHAR  DateTimeOriginalformat[2]={0x2,0x0};
    UCHAR  NcomponentDateTimeOriginal[4]={20,0,0,0};
    UCHAR  DateTimeOriginal[32];//="2006:6:09 15:17:32";
    char  DateTimeOriginalClose[1]={0};
    UCHAR DateTimeOriginaloffchar[4];

    //VARIABLES FOR THE ISO SPEED RATINGS OF THE CAMERA
    UCHAR  ISOSpeedRatingstag[2]={0x27,0x88};
    UCHAR  ISOSpeedRatingsformat[2]={0x3,0x0};
    UCHAR  NcomponentISOSpeedRatings[4]={0x2,0x0,0x0,0x0};
    unsigned short   ISOSpeedRatings[2];
    UCHAR  ISOSpeedRatingsChar[4] = {0};

    //VARIABLES FOR THE BRIGHTNESS OF THE IMAGE
    UCHAR  Brightnesstag[2]={0x03,0x92};
    UCHAR  Brightnessformat[2]={0xA,0x0};
    UCHAR  NcomponentBrightness[4]={0x1,0x0,0x0,0x0};
    int BrightnessNum[1];
    int BrightnessDen[1];

    UCHAR Brightnessoffchar[4];
    UCHAR BrightnessNumChar[4];
    UCHAR BrightnessDenChar[4];

    //VARIABLES FOR THE EXPOSURE Bias
    UCHAR  ExposureBiastag[2]={0x04,0x92};
    UCHAR  ExposureBiasformat[2]={0xA,0x0};
    UCHAR  NcomponentExposureBias[4]={0x1,0x0,0x0,0x0};
    int ExposureBiasNum[1];//={-8};
    int ExposureBiasDen[1];//={1};

    UCHAR ExposureBiasoffchar[4];
    UCHAR ExposureBiasNumChar[4];
    UCHAR ExposureBiasDenChar[4];

    //VARIABLES FOR THE SUBJECT DISTANCE OF THE IMAGE
    UCHAR  SubjectDistancetag[2]={0x06,0x92};
    UCHAR  SubjectDistanceformat[2]={0xA,0x0};
    UCHAR  NcomponentSubjectDistance[4]={0x1,0x0,0x0,0x0};
    int SubjectDistanceNum[1];
    int SubjectDistanceDen[1];

    UCHAR SubjectDistanceoffchar[4];
    UCHAR SubjectDistanceNumChar[4];
    UCHAR SubjectDistanceDenChar[4];

    //VARIABLES FOR THE METERING MODE
    UCHAR  MeteringModetag[2]={0x07,0x92};
    UCHAR  MeteringModeformat[2]={0x3,0x0};
    UCHAR  NcomponentMeteringMode[4]={0x1,0x0,0x0,0x0};
    UINT   MeteringMode[1];
    UCHAR  MeteringModeChar[4] = {0};

    //VARIABLES FOR THE FLASH
    UCHAR  Flashtag[2]={0x09,0x92};
    UCHAR  Flashformat[2]={0x3,0x0};
    UCHAR  NcomponentFlash[4]={0x1,0x0,0x0,0x0};
    UINT   Flash[1]={1};
    UCHAR  FlashChar[4] = {0};

    //VARIABLES FOR THE FOCAL LENGTH
    UCHAR  FocalLengthtag[2]={0x0A,0x92};
    UCHAR  FocalLengthformat[2]={0x5,0x0};
    UCHAR  NcomponentFocalLength[4]={0x1,0x0,0x0,0x0};
    UINT FocalLengthNum[1];
    UINT FocalLengthDen[1];

    UCHAR FocalLengthoffchar[4];
    UCHAR FocalLengthNumChar[4];
    UCHAR FocalLengthDenChar[4];

    //VARIABLES FOR THE ISO WIDTH OF THE MAIN IMAGE
    UCHAR  Widthtag[2]={0x02,0xA0};
    UCHAR  Widthformat[2]={0x3,0x0};
    UCHAR  NcomponentWidth[4]={0x1,0x0,0x0,0x0};
    UINT   Width[1];
    UCHAR  WidthChar[4] = {0};

    //VARIABLES FOR THE ISO HEIGHT OF THE MAIN IMAGE
    UCHAR  Heighttag[2]={0x03,0xA0};
    UCHAR  Heightformat[2]={0x3,0x0};
    UCHAR  NcomponentHeight[4]={0x1,0x0,0x0,0x0};
    UINT   Height[1];
    UCHAR  HeightChar[4] = {0};

    //VARIABLES FOR THE COLORSPACE
    UCHAR  ColorSpacetag[2]={0x01,0xA0};
    //char  ColorSpacetag[2]={0x54,0x56};
    UCHAR  ColorSpaceformat[2]={0x3,0x0};
    UCHAR  NcomponentColorSpace[4]={0x1,0x0,0x0,0x0};
    UINT   ColorSpace[1];//={1};
    UCHAR  ColorSpaceChar[4] = {0};

    //VARIABLES FOR THE FocalPlaneXResolution
    UCHAR  FocalPlaneXResolutiontag[2]={0x0E,0xA2};
    UCHAR  FocalPlaneXResolutionformat[2]={0x5,0x0};
    UCHAR  NcomponentFocalPlaneXResolution[4]={0x1,0x0,0x0,0x0};
    UINT FocalPlaneXResolutionNum[1];
    UINT FocalPlaneXResolutionDen[1];

    UCHAR FocalPlaneXResolutionoffchar[4];
    UCHAR FocalPlaneXResolutionNumChar[4];
    UCHAR FocalPlaneXResolutionDenChar[4];

    //VARIABLES FOR THE FocalPlaneYResolution
    UCHAR  FocalPlaneYResolutiontag[2]={0x0F,0xA2};
    UCHAR  FocalPlaneYResolutionformat[2]={0x5,0x0};
    UCHAR  NcomponentFocalPlaneYResolution[4]={0x1,0x0,0x0,0x0};
    UINT FocalPlaneYResolutionNum[1];
    UINT FocalPlaneYResolutionDen[1];

    UCHAR FocalPlaneYResolutionoffchar[4];
    UCHAR FocalPlaneYResolutionNumChar[4];
    UCHAR FocalPlaneYResolutionDenChar[4];

    //VARIABLES FOR THE FocalPlaneResolutionUnit
    UCHAR  FocalPlaneResolutionUnittag[2]={0x10,0xA2};
    UCHAR  FocalPlaneResolutionUnitformat[2]={0x3,0x0};
    UCHAR  NcomponentFocalPlaneResolutionUnit[4]={0x1,0x0,0x0,0x0};
    UINT   FocalPlaneResolutionUnit[1];
    UCHAR  FocalPlaneResolutionUnitChar[4] = {0};


    //VARIABLES FOR THE WHITE BALANCE PROGRAM OF THE CAMERA
    UCHAR  WhiteBalancetag[2]={0x03,0xA4};
    UCHAR  WhiteBalanceformat[2]={0x3,0x0};
    UCHAR  NcomponentWhiteBalance[4]={0x1,0x0,0x0,0x0};
    UINT WhiteBalance[1];
    UCHAR  WhiteBalanceChar[4] = {0};

    //VARIABLES FOR THE USER COMMENTS
    UCHAR  UserCommentstag[2]={0x86,0x92};
    UCHAR  UserCommentsformat[2]={0x7,0x0};
    UCHAR  NcomponentUserComments[4]={150,0x0,0x0,0x0};
    UCHAR  UserComments[150];
    UCHAR UserCommentsoffchar[4];

    //VARIABLES FOR THE COMPRESSION TYPE
    UCHAR  Compressiontag[2]={0x03,0x01};
    UCHAR  Compressionformat[2]={0x3,0x0};
    UCHAR  NcomponentCompression[4]={0x1,0x0,0x0,0x0};
    UINT   Compression[1]={6};
    UCHAR  CompressionChar[4] = {0};

    //VARIABLES FOR THE JpegIFOffset
    UCHAR  JpegIFOffsettag[2]={0x01,0x02};
    UCHAR  JpegIFOffsetformat[2]={0x4,0x0};
    UCHAR  NcomponentJpegIFOffset[4]={0x1,0x0,0x0,0x0};
    UCHAR  JpegIFOffsetChar[4] = {0};

    //VARIABLES FOR THE JpegIFByteCount
    UCHAR  JpegIFByteCounttag[2]={0x02,0x02};
    UCHAR  JpegIFByteCountformat[2]={0x4,0x0};
    UCHAR  NcomponentJpegIFByteCount[4]={0x1,0x0,0x0,0x0};
    UCHAR  JpegIFByteCountChar[4] = {0};
    //END OF THE VARIABLES

    ExifInitialCount=tempExif;
    //for APP1 Marker(2 byte) and length(2 byte)
    tempExif += 4;
    //write an exif header
    memcpy ( tempExif, ExifHeader, 6 );
    tempExif += 6 ;

    //write a tiff header
    memcpy ( tempExif, TIFFHeader, 8 );
    startoftiff=tempExif;
    tempExif += 8 ;
    //write no of entries in 1d0
    memcpy ( tempExif, Nentries, 2 );
    tempExif += 2 ;
    ///////////////ENTRY NO 1 :MAKE OF CAMERA////////////////////////
    //write make tag
    memcpy ( tempExif, maketag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, makeformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, Ncomponent, 4 );
    tempExif += 4 ;
    //write make
    //strcpy(make,tpJEInfo->Make);
    memcpy ( make, exifFileInfo->Make,32 );
    offset = ( UCHAR * ) 0x200;
    santemp= ( int ) ( offset );
    makeoffchar[0]= ( unsigned char ) santemp;
    makeoffchar[1]= ( unsigned char ) ( santemp>>8 );
    makeoffchar[2]= ( unsigned char ) ( santemp>>16 );
    makeoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the make offset into the bitstream
    memcpy ( tempExif, makeoffchar, 4 );
    tempExif += 4 ;
    memcpy ( startoftiff+santemp, make, 32 );
    offset+=32;

    ///////////////ENTRY NO 2 :MODEL OF CAMERA////////////////////////
    //write model tag
    memcpy ( tempExif, modeltag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, modelformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentModel, 4 ); //sanjeev
    tempExif += 4 ;
    //write model
    //	strcpy(model,tpJEInfo->Model);
    memcpy ( model,exifFileInfo->Model,32 );
    santemp= ( int ) ( offset );
    modeloffchar[0]= ( unsigned char ) santemp;
    modeloffchar[1]= ( unsigned char ) ( santemp>>8 );
    modeloffchar[2]= ( unsigned char ) ( santemp>>16 );
    modeloffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the model offset into the bitstream
    memcpy ( tempExif, modeloffchar, 4 );
    tempExif += 4 ;
    memcpy ( startoftiff+santemp, model, 32 );
    offset+=32;


    ///////////////ENTRY NO 3 :ORIENTATION OF CAMERA////////////////////////
    //write orientation tag
    memcpy ( tempExif, orientationtag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, orientationformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentOrientation, 4 );
    tempExif += 4 ;
    //write orientation mode
    Orientation[0] = exifFileInfo->Orientation;
    Orient[0] = ( unsigned char ) ( Orientation[0] );
    Orient[1] = ( unsigned char ) ( Orientation[0]>>8 );
    Orient[2] = ( unsigned char ) ( Orientation[0]>>16 );
    Orient[3] = ( unsigned char ) ( Orientation[0]>>24 );

    memcpy ( tempExif, Orient, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 4 :JPEG PROCESS////////////////////////
    //write orientation tag
    memcpy ( tempExif, Processtag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, Processformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentProcess, 4 );
    tempExif += 4 ;
    //write orientation mode
    Process[0] =exifFileInfo->Process;
    Proc[0] = ( unsigned char ) ( Process[0] );
    Proc[1] = ( unsigned char ) ( Process[0]>>8 );
    Proc[2] = ( unsigned char ) ( Process[0]>>16 );
    Proc[3] = ( unsigned char ) ( Process[0]>>24 );

    memcpy ( tempExif, Proc, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 5 :VERSION OF software////////////////////////
    //write VERSION tag
    memcpy ( tempExif, Versiontag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, Versionformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentVersion, 4 ); //sanjeev
    tempExif += 4 ;


    santemp= ( int ) ( offset );
    Versionoffchar[0]= ( unsigned char ) santemp;
    Versionoffchar[1]= ( unsigned char ) ( santemp>>8 );
    Versionoffchar[2]= ( unsigned char ) ( santemp>>16 );
    Versionoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the VERSION offset into the bitstream
    memcpy ( tempExif, Versionoffchar, 4 );
    tempExif += 4 ;
    //	strcpy(Version,jCtx->ExifInfo->Version);
    memcpy ( Version,exifFileInfo->Version,32 );
    memcpy ( startoftiff+santemp, Version, 32 );
    offset+=32;

    ///////////////ENTRY NO 6 :Date/Time////////////////////////
    //write model tag
    memcpy ( tempExif, DateTimetag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, DateTimeformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentDateTime, 4 ); //sanjeev
    tempExif += 4 ;
    //write Date/Time
    //strcpy(DateTime,jCtx->ExifInfo->DateTime);
    memcpy ( DateTime,exifFileInfo->DateTime,20 );

    santemp= ( int ) ( offset );
    DateTimeoffchar[0]= ( unsigned char ) santemp;
    DateTimeoffchar[1]= ( unsigned char ) ( santemp>>8 );
    DateTimeoffchar[2]= ( unsigned char ) ( santemp>>16 );
    DateTimeoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the model offset into the bitstream
    memcpy ( tempExif, DateTimeoffchar, 4 );
    tempExif += 4 ;
    memcpy ( startoftiff+santemp, DateTime, 19 );
    memcpy ( startoftiff+santemp+19, DateTimeClose, 1 );

    offset+=32;
    ///////////////ENTRY NO 7 :COPYRIGHT INFO////////////////////////
    //write model tag
    memcpy ( tempExif, CopyRighttag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, CopyRightformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentCopyRight, 4 ); //sanjeev
    tempExif += 4 ;

    //	strcpy(CopyRight,jCtx->ExifInfo->CopyRight);////="copyright 2006";);
    memcpy ( CopyRight,exifFileInfo->CopyRight,32 );

    santemp= ( int ) ( offset );
    CopyRightoffchar[0]= ( unsigned char ) santemp;
    CopyRightoffchar[1]= ( unsigned char ) ( santemp>>8 );
    CopyRightoffchar[2]= ( unsigned char ) ( santemp>>16 );
    CopyRightoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the model offset into the bitstream
    memcpy ( tempExif, CopyRightoffchar, 4 );
    tempExif += 4 ;
    memcpy ( startoftiff+santemp, CopyRight, 32 );

    offset+=32;
    ///////////////ENTRY NO 8 :OFFSET TO THE SubIFD ////////////////////////
    //write SubIFD tag
    memcpy ( tempExif, SubIFDOffsettag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, SubIFDOffsetformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentSubIFDOffset, 4 );
    tempExif += 4 ;
    //write the  offset to the SubIFD
    santemp= ( int ) ( tempExif-startoftiff+8 );
    SubIFDOffsetChar[0] = ( unsigned char ) ( santemp );
    SubIFDOffsetChar[1] = ( unsigned char ) ( santemp>>8 );
    SubIFDOffsetChar[2] = ( unsigned char ) ( santemp>>16 );
    SubIFDOffsetChar[3] = ( unsigned char ) ( santemp>>24 );

    memcpy ( tempExif, SubIFDOffsetChar, 4 );
    tempExif += 4 ;


    // since it was the last directory entry, so next 4 bytes contains an offset to the IFD1.

    //since we dont know the offset lets put 0x0000 as an offset, later when get to know the
    //actual offset we will revisit here and put the actual offset.
    santemp=0x0000;
    SubIFDOffsetChar[0] = ( unsigned char ) ( santemp );
    SubIFDOffsetChar[1] = ( unsigned char ) ( santemp>>8 );
    SubIFDOffsetChar[2] = ( unsigned char ) ( santemp>>16 );
    SubIFDOffsetChar[3] = ( unsigned char ) ( santemp>>24 );
    IFD1OffsetAddress = tempExif;
    memcpy ( tempExif, SubIFDOffsetChar, 4 );
    tempExif += 4 ;
    /////////////EXIF SUBIFD STARTS HERE//////////////////////////////////
    //write no of entries in SubIFD
    memcpy ( tempExif, SubIFDNentries, 2 );
    tempExif += 2 ;

    ///////////////ENTRY NO 1 :Original/Time////////////////////////
    //write Original/Time tag
    memcpy ( tempExif, DateTimeOriginaltag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, DateTimeOriginalformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentDateTimeOriginal, 4 ); //sanjeev
    tempExif += 4 ;
    //write Original/Time
    //strcpy(DateTimeOriginal,jCtx->ExifInfo->DateTimeOriginal);
    memcpy ( DateTimeOriginal,exifFileInfo->DateTimeOriginal,20 );

    santemp= ( int ) ( offset );
    DateTimeOriginaloffchar[0]= ( unsigned char ) santemp;
    DateTimeOriginaloffchar[1]= ( unsigned char ) ( santemp>>8 );
    DateTimeOriginaloffchar[2]= ( unsigned char ) ( santemp>>16 );
    DateTimeOriginaloffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the model offset into the bitstream
    memcpy ( tempExif, DateTimeOriginaloffchar, 4 );
    tempExif += 4 ;
    memcpy ( startoftiff+santemp, DateTimeOriginal, 19 );
    memcpy ( startoftiff+santemp+19, DateTimeOriginalClose, 1 );

    offset+=32;

    ///////////////ENTRY NO 2 :Width////////////////////////
    //write Width tag
    memcpy ( tempExif, Widthtag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, Widthformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentWidth, 4 );
    tempExif += 4 ;
    //write Width
    Width[0]=exifFileInfo->Width;
    WidthChar[0] = ( unsigned char ) ( Width[0] );
    WidthChar[1] = ( unsigned char ) ( Width[0]>>8 );
    WidthChar[2] = ( unsigned char ) ( Width[0]>>16 );
    WidthChar[3] = ( unsigned char ) ( Width[0]>>24 );

    memcpy ( tempExif, WidthChar, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 3 :Height////////////////////////
    //write Height tag
    memcpy ( tempExif, Heighttag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, Heightformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentHeight, 4 );
    tempExif += 4 ;
    //write Height
    Height[0]=exifFileInfo->Height;
    HeightChar[0] = ( unsigned char ) ( Height[0] );
    HeightChar[1] = ( unsigned char ) ( Height[0]>>8 );
    HeightChar[2] = ( unsigned char ) ( Height[0]>>16 );
    HeightChar[3] = ( unsigned char ) ( Height[0]>>24 );

    memcpy ( tempExif, HeightChar, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 4 :WHITE BALANCE////////////////////////
    //write WhiteBalance tag
    memcpy ( tempExif, WhiteBalancetag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, WhiteBalanceformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentWhiteBalance, 4 );
    tempExif += 4 ;
    //write orientation mode
    WhiteBalance[0] = exifFileInfo->WhiteBalance;
    WhiteBalanceChar[0] = ( unsigned char ) ( WhiteBalance[0] );
    WhiteBalanceChar[1] = ( unsigned char ) ( WhiteBalance[0]>>8 );
    WhiteBalanceChar[2] = ( unsigned char ) ( WhiteBalance[0]>>16 );
    WhiteBalanceChar[3] = ( unsigned char ) ( WhiteBalance[0]>>24 );

    memcpy ( tempExif, WhiteBalanceChar, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 5 :EXPOSURE PROGRAM////////////////////////
    //write ExposureProgram tag
    memcpy ( tempExif, ExposureProgramtag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, ExposureProgramformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentExposureProgram, 4 );
    tempExif += 4 ;
    //write orientation mode
    ExposureProgram[0] =exifFileInfo->ExposureProgram;

    ExposureProgramChar[0] = ( unsigned char ) ( ExposureProgram[0] );
    ExposureProgramChar[1] = ( unsigned char ) ( ExposureProgram[0]>>8 );
    ExposureProgramChar[2] = ( unsigned char ) ( ExposureProgram[0]>>16 );
    ExposureProgramChar[3] = ( unsigned char ) ( ExposureProgram[0]>>24 );

    memcpy ( tempExif, ExposureProgramChar, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 6 : EXPOSURE TIME////////////////////////
    //write EXPOSURE TIME tag
    memcpy ( tempExif, ExposureTimetag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, ExposureTimeformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentExposureTime, 4 );
    tempExif += 4 ;
    //write EXPOSURE TIME

    santemp= ( int ) ( offset );
    ExposureTimeoffchar[0]= ( unsigned char ) santemp;
    ExposureTimeoffchar[1]= ( unsigned char ) ( santemp>>8 );
    ExposureTimeoffchar[2]= ( unsigned char ) ( santemp>>16 );
    ExposureTimeoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the X-Resolution offset into the bitstream
    memcpy ( tempExif, ExposureTimeoffchar, 4 );
    tempExif += 4 ;

    ExposureTimeNum[0]=exifFileInfo->ExposureTimeNum;
    ExposureTimeDen[0]=exifFileInfo->ExposureTimeDen;
    ExposureTimeNumChar[0]= ( unsigned char ) ExposureTimeNum[0];
    ExposureTimeNumChar[1]= ( unsigned char ) ( ExposureTimeNum[0]>>8 );
    ExposureTimeNumChar[2]= ( unsigned char ) ( ExposureTimeNum[0]>>16 );
    ExposureTimeNumChar[3]= ( unsigned char ) ( ExposureTimeNum[0]>>24 );

    ExposureTimeDenChar[0]= ( unsigned char ) ExposureTimeDen[0];
    ExposureTimeDenChar[1]= ( unsigned char ) ( ExposureTimeDen[0]>>8 );
    ExposureTimeDenChar[2]= ( unsigned char ) ( ExposureTimeDen[0]>>16 );
    ExposureTimeDenChar[3]= ( unsigned char ) ( ExposureTimeDen[0]>>24 );

    //WRITE THE EXPOSURE TIME NUMERATOR
    memcpy ( startoftiff+santemp, ExposureTimeNumChar, 4 );

    memcpy ( startoftiff+santemp+4, ExposureTimeDenChar, 4 );

    offset+=32;

    ///////////////ENTRY NO 7 : EXPOSURE BIAS OF THE IMAGE////////////////////////
    //write BRIGHTNESS tag
    memcpy ( tempExif, ExposureBiastag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, ExposureBiasformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentExposureBias, 4 ); //sanjeev
    tempExif += 4 ;
    //write EXPOSURE BIAS


    santemp= ( int ) ( offset );
    ExposureBiasoffchar[0]= ( unsigned char ) santemp;
    ExposureBiasoffchar[1]= ( unsigned char ) ( santemp>>8 );
    ExposureBiasoffchar[2]= ( unsigned char ) ( santemp>>16 );
    ExposureBiasoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the EXPOSURE BIAS offset into the bitstream
    memcpy ( tempExif, ExposureBiasoffchar, 4 );
    tempExif += 4 ;
    ExposureBiasNum[0]=exifFileInfo->ExposureBiasNum;
    ExposureBiasDen[0]=exifFileInfo->ExposureBiasDen;
    ExposureBiasNumChar[0]= ( unsigned char ) ExposureBiasNum[0];
    ExposureBiasNumChar[1]= ( unsigned char ) ( ExposureBiasNum[0]>>8 );
    ExposureBiasNumChar[2]= ( unsigned char ) ( ExposureBiasNum[0]>>16 );
    ExposureBiasNumChar[3]= ( unsigned char ) ( ExposureBiasNum[0]>>24 );

    ExposureBiasDenChar[0]= ( unsigned char ) ExposureBiasDen[0];
    ExposureBiasDenChar[1]= ( unsigned char ) ( ExposureBiasDen[0]>>8 );
    ExposureBiasDenChar[2]= ( unsigned char ) ( ExposureBiasDen[0]>>16 );
    ExposureBiasDenChar[3]= ( unsigned char ) ( ExposureBiasDen[0]>>24 );

    //WRITE THE EXPOSURE BIAS NUMERATOR
    memcpy ( startoftiff+santemp, ExposureBiasNumChar, 4 );

    memcpy ( startoftiff+santemp+4, ExposureBiasDenChar, 4 );

    offset+=48;

    ///////////////ENTRY NO 8 : F NUMBER////////////////////////
    //write FNumber tag
    memcpy ( tempExif, FNumbertag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, FNumberformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentFNumber, 4 ); //sanjeev
    tempExif += 4 ;
    //write F NUMBER

    santemp= ( int ) ( offset );
    FNumberoffchar[0]= ( unsigned char ) santemp;
    FNumberoffchar[1]= ( unsigned char ) ( santemp>>8 );
    FNumberoffchar[2]= ( unsigned char ) ( santemp>>16 );
    FNumberoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the F NUMBER into the bitstream
    memcpy ( tempExif, FNumberoffchar, 4 );
    tempExif += 4 ;

    FNumberNum[0]=exifFileInfo->FNumberNum;
    FNumberDen[0]=exifFileInfo->FNumberDen;

    FNumberNumChar[0]= ( unsigned char ) FNumberNum[0];
    FNumberNumChar[1]= ( unsigned char ) ( FNumberNum[0]>>8 );
    FNumberNumChar[2]= ( unsigned char ) ( FNumberNum[0]>>16 );
    FNumberNumChar[3]= ( unsigned char ) ( FNumberNum[0]>>24 );

    FNumberDenChar[0]= ( unsigned char ) FNumberDen[0];
    FNumberDenChar[1]= ( unsigned char ) ( FNumberDen[0]>>8 );
    FNumberDenChar[2]= ( unsigned char ) ( FNumberDen[0]>>16 );
    FNumberDenChar[3]= ( unsigned char ) ( FNumberDen[0]>>24 );

    //WRITE THE FNumber NUMERATOR
    memcpy ( startoftiff+santemp, FNumberNumChar, 4 );

    memcpy ( startoftiff+santemp+4, FNumberDenChar, 4 );

    offset+=32;


    ///////////////ENTRY NO 9 :ISOSpeedRatings ////////////////////////
    //write ISOSpeedRatings  tag
    memcpy ( tempExif, ISOSpeedRatingstag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, ISOSpeedRatingsformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentISOSpeedRatings, 4 );
    tempExif += 4 ;
    //write orientation mode
    ISOSpeedRatings[0] = 1;
    ISOSpeedRatings[1] = 2;

    ISOSpeedRatingsChar[0] = ( unsigned char ) ( ISOSpeedRatings[0] );
    ISOSpeedRatingsChar[1] = ( unsigned char ) ( ISOSpeedRatings[0]>>8 );
    ISOSpeedRatingsChar[2] = ( unsigned char ) ( ISOSpeedRatings[1] );
    ISOSpeedRatingsChar[3] = ( unsigned char ) ( ISOSpeedRatings[1]>>8 );

    memcpy ( tempExif, ISOSpeedRatingsChar, 4 );
    tempExif += 4 ;


    ///////////////ENTRY NO 10 : BRIGHTNESS OF THE IMAGE////////////////////////
    //write BRIGHTNESS tag
    memcpy ( tempExif, Brightnesstag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, Brightnessformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentBrightness, 4 ); //sanjeev
    tempExif += 4 ;
    //write X - Resolution

    santemp= ( int ) ( offset );
    Brightnessoffchar[0]= ( unsigned char ) santemp;
    Brightnessoffchar[1]= ( unsigned char ) ( santemp>>8 );
    Brightnessoffchar[2]= ( unsigned char ) ( santemp>>16 );
    Brightnessoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the X-Resolution offset into the bitstream
    memcpy ( tempExif, Brightnessoffchar, 4 );
    tempExif += 4 ;

    BrightnessNum[0] = exifFileInfo->BrightnessNum;
    BrightnessDen[0] = exifFileInfo->BrightnessDen;

    BrightnessNumChar[0]= ( unsigned char ) BrightnessNum[0];
    BrightnessNumChar[1]= ( unsigned char ) ( BrightnessNum[0]>>8 );
    BrightnessNumChar[2]= ( unsigned char ) ( BrightnessNum[0]>>16 );
    BrightnessNumChar[3]= ( unsigned char ) ( BrightnessNum[0]>>24 );

    BrightnessDenChar[0]= ( unsigned char ) BrightnessDen[0];
    BrightnessDenChar[1]= ( unsigned char ) ( BrightnessDen[0]>>8 );
    BrightnessDenChar[2]= ( unsigned char ) ( BrightnessDen[0]>>16 );
    BrightnessDenChar[3]= ( unsigned char ) ( BrightnessDen[0]>>24 );

    //WRITE THE X - RESOLUTION NUMERATOR
    memcpy ( startoftiff+santemp, BrightnessNumChar, 4 );
    memcpy ( startoftiff+santemp+4, BrightnessDenChar, 4 );
    offset+=48;

    ///////////////ENTRY NO 11 : SUBJECT DISTANCE////////////////////////
    //write SUBJECT DISTANCE tag
    memcpy ( tempExif, SubjectDistancetag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, SubjectDistanceformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentSubjectDistance, 4 ); //sanjeev
    tempExif += 4 ;
    //write SUBJECT DISTANCE


    santemp= ( int ) ( offset );
    SubjectDistanceoffchar[0]= ( unsigned char ) santemp;
    SubjectDistanceoffchar[1]= ( unsigned char ) ( santemp>>8 );
    SubjectDistanceoffchar[2]= ( unsigned char ) ( santemp>>16 );
    SubjectDistanceoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the SUBJECT DISTANCE offset into the bitstream
    memcpy ( tempExif, SubjectDistanceoffchar, 4 );
    tempExif += 4 ;
    SubjectDistanceNum[0]=exifFileInfo->SubjectDistanceNum;
    SubjectDistanceDen[0]=exifFileInfo->SubjectDistanceDen;
    SubjectDistanceNumChar[0]= ( unsigned char ) SubjectDistanceNum[0];
    SubjectDistanceNumChar[1]= ( unsigned char ) ( SubjectDistanceNum[0]>>8 );
    SubjectDistanceNumChar[2]= ( unsigned char ) ( SubjectDistanceNum[0]>>16 );
    SubjectDistanceNumChar[3]= ( unsigned char ) ( SubjectDistanceNum[0]>>24 );

    SubjectDistanceDenChar[0]= ( unsigned char ) SubjectDistanceDen[0];
    SubjectDistanceDenChar[1]= ( unsigned char ) ( SubjectDistanceDen[0]>>8 );
    SubjectDistanceDenChar[2]= ( unsigned char ) ( SubjectDistanceDen[0]>>16 );
    SubjectDistanceDenChar[3]= ( unsigned char ) ( SubjectDistanceDen[0]>>24 );

    //WRITE THE SUBJECT DISTANCE NUMERATOR
    memcpy ( startoftiff+santemp, SubjectDistanceNumChar, 4 );

    memcpy ( startoftiff+santemp+4, SubjectDistanceDenChar, 4 );

    offset+=48;

    ///////////////ENTRY NO 12 :METERING MODE////////////////////////
    //write METERING tag
    memcpy ( tempExif, MeteringModetag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, MeteringModeformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentMeteringMode, 4 );
    tempExif += 4 ;
    //write METERING mode
    MeteringMode[0] = exifFileInfo->MeteringMode;
    MeteringModeChar[0] = ( unsigned char ) ( MeteringMode[0] );
    MeteringModeChar[1] = ( unsigned char ) ( MeteringMode[0]>>8 );
    MeteringModeChar[2] = ( unsigned char ) ( MeteringMode[0]>>16 );
    MeteringModeChar[3] = ( unsigned char ) ( MeteringMode[0]>>24 );

    memcpy ( tempExif, MeteringModeChar, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 13 :FLASH////////////////////////
    //write FLASH tag
    memcpy ( tempExif, Flashtag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, Flashformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentFlash, 4 );
    tempExif += 4 ;
    //write FLASH mode
    Flash[0]= exifFileInfo->Flash;
    FlashChar[0] = ( unsigned char ) ( Flash[0] );
    FlashChar[1] = ( unsigned char ) ( Flash[0]>>8 );
    FlashChar[2] = ( unsigned char ) ( Flash[0]>>16 );
    FlashChar[3] = ( unsigned char ) ( Flash[0]>>24 );

    memcpy ( tempExif, FlashChar, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 14 : FOCAL LENGTH////////////////////////
    //write FOCAL LENGTH tag
    memcpy ( tempExif, FocalLengthtag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, FocalLengthformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentFocalLength, 4 ); //sanjeev
    tempExif += 4 ;
    //write FOCAL LENGTH

    santemp= ( int ) ( offset );
    FocalLengthoffchar[0]= ( unsigned char ) santemp;
    FocalLengthoffchar[1]= ( unsigned char ) ( santemp>>8 );
    FocalLengthoffchar[2]= ( unsigned char ) ( santemp>>16 );
    FocalLengthoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the FOCAL LENGTH offset into the bitstream
    memcpy ( tempExif, FocalLengthoffchar, 4 );
    tempExif += 4 ;
    FocalLengthNum[0]=exifFileInfo->FocalLengthNum;
    FocalLengthDen[0]=exifFileInfo->FocalLengthDen;
    FocalLengthNumChar[0]= ( unsigned char ) FocalLengthNum[0];
    FocalLengthNumChar[1]= ( unsigned char ) ( FocalLengthNum[0]>>8 );
    FocalLengthNumChar[2]= ( unsigned char ) ( FocalLengthNum[0]>>16 );
    FocalLengthNumChar[3]= ( unsigned char ) ( FocalLengthNum[0]>>24 );

    FocalLengthDenChar[0]= ( unsigned char ) FocalLengthDen[0];
    FocalLengthDenChar[1]= ( unsigned char ) ( FocalLengthDen[0]>>8 );
    FocalLengthDenChar[2]= ( unsigned char ) ( FocalLengthDen[0]>>16 );
    FocalLengthDenChar[3]= ( unsigned char ) ( FocalLengthDen[0]>>24 );

    //WRITE THE FOCAL LENGTH NUMERATOR
    memcpy ( startoftiff+santemp, FocalLengthNumChar, 4 );

    memcpy ( startoftiff+santemp+4, FocalLengthDenChar, 4 );

    offset+=48;

    ///////////////ENTRY NO 15 :COLORSPACE////////////////////////
    //write ExposureProgram tag
    memcpy ( tempExif, ColorSpacetag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, ColorSpaceformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentColorSpace, 4 );
    tempExif += 4 ;
    //write orientation mode
    ColorSpace [0]= exifFileInfo->ColorSpace;
    ColorSpaceChar[0] = ( unsigned char ) ( ColorSpace[0] );
    ColorSpaceChar[1] = ( unsigned char ) ( ColorSpace[0]>>8 );
    ColorSpaceChar[2] = ( unsigned char ) ( ColorSpace[0]>>16 );
    ColorSpaceChar[3] = ( unsigned char ) ( ColorSpace[0]>>24 );

    memcpy ( tempExif, ColorSpaceChar, 4 );
    tempExif += 4 ;
    ///////////////ENTRY NO 16 : FocalPlaneXResolution////////////////////////
    //write EXPOSURE TIME tag
    memcpy ( tempExif, FocalPlaneXResolutiontag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, FocalPlaneXResolutionformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentFocalPlaneXResolution, 4 );
    tempExif += 4 ;
    //write EXPOSURE TIME

    santemp= ( int ) ( offset );
    FocalPlaneXResolutionoffchar[0]= ( unsigned char ) santemp;
    FocalPlaneXResolutionoffchar[1]= ( unsigned char ) ( santemp>>8 );
    FocalPlaneXResolutionoffchar[2]= ( unsigned char ) ( santemp>>16 );
    FocalPlaneXResolutionoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the X-Resolution offset into the bitstream
    memcpy ( tempExif, FocalPlaneXResolutionoffchar, 4 );
    tempExif += 4 ;
    FocalPlaneXResolutionNum[0] = exifFileInfo->FocalPlaneXResolutionNum;
    FocalPlaneXResolutionDen[0] = exifFileInfo->FocalPlaneXResolutionDen;

    FocalPlaneXResolutionNumChar[0]= ( unsigned char ) FocalPlaneXResolutionNum[0];
    FocalPlaneXResolutionNumChar[1]= ( unsigned char ) ( FocalPlaneXResolutionNum[0]>>8 );
    FocalPlaneXResolutionNumChar[2]= ( unsigned char ) ( FocalPlaneXResolutionNum[0]>>16 );
    FocalPlaneXResolutionNumChar[3]= ( unsigned char ) ( FocalPlaneXResolutionNum[0]>>24 );

    FocalPlaneXResolutionDenChar[0]= ( unsigned char ) FocalPlaneXResolutionDen[0];
    FocalPlaneXResolutionDenChar[1]= ( unsigned char ) ( FocalPlaneXResolutionDen[0]>>8 );
    FocalPlaneXResolutionDenChar[2]= ( unsigned char ) ( FocalPlaneXResolutionDen[0]>>16 );
    FocalPlaneXResolutionDenChar[3]= ( unsigned char ) ( FocalPlaneXResolutionDen[0]>>24 );

    //WRITE THE EXPOSURE TIME NUMERATOR
    memcpy ( startoftiff+santemp, FocalPlaneXResolutionNumChar, 4 );

    memcpy ( startoftiff+santemp+4, FocalPlaneXResolutionDenChar, 4 );

    offset+=48;

    ///////////////ENTRY NO 17 : FocalPlaneYResolution////////////////////////
    //write EXPOSURE TIME tag
    memcpy ( tempExif, FocalPlaneYResolutiontag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, FocalPlaneYResolutionformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentFocalPlaneYResolution, 4 ); //sanjeev
    tempExif += 4 ;
    //write EXPOSURE TIME

    santemp= ( int ) ( offset );
    FocalPlaneYResolutionoffchar[0]= ( unsigned char ) santemp;
    FocalPlaneYResolutionoffchar[1]= ( unsigned char ) ( santemp>>8 );
    FocalPlaneYResolutionoffchar[2]= ( unsigned char ) ( santemp>>16 );
    FocalPlaneYResolutionoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the X-Resolution offset into the bitstream
    memcpy ( tempExif, FocalPlaneYResolutionoffchar, 4 );
    tempExif += 4 ;
    FocalPlaneYResolutionNum[0] = exifFileInfo->FocalPlaneYResolutionNum;
    FocalPlaneYResolutionDen[0] = exifFileInfo->FocalPlaneYResolutionDen;

    FocalPlaneYResolutionNumChar[0]= ( unsigned char ) FocalPlaneYResolutionNum[0];
    FocalPlaneYResolutionNumChar[1]= ( unsigned char ) ( FocalPlaneYResolutionNum[0]>>8 );
    FocalPlaneYResolutionNumChar[2]= ( unsigned char ) ( FocalPlaneYResolutionNum[0]>>16 );
    FocalPlaneYResolutionNumChar[3]= ( unsigned char ) ( FocalPlaneYResolutionNum[0]>>24 );

    FocalPlaneYResolutionDenChar[0]= ( unsigned char ) FocalPlaneYResolutionDen[0];
    FocalPlaneYResolutionDenChar[1]= ( unsigned char ) ( FocalPlaneYResolutionDen[0]>>8 );
    FocalPlaneYResolutionDenChar[2]= ( unsigned char ) ( FocalPlaneYResolutionDen[0]>>16 );
    FocalPlaneYResolutionDenChar[3]= ( unsigned char ) ( FocalPlaneYResolutionDen[0]>>24 );

    //WRITE THE EXPOSURE TIME NUMERATOR
    memcpy ( startoftiff+santemp, FocalPlaneYResolutionNumChar, 4 );

    memcpy ( startoftiff+santemp+4, FocalPlaneYResolutionDenChar, 4 );

    offset+=48;

    ///////////////ENTRY NO 18 :FocalPlaneResolutionUnit////////////////////////
    //write ExposureProgram tag
    memcpy ( tempExif, FocalPlaneResolutionUnittag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, FocalPlaneResolutionUnitformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentFocalPlaneResolutionUnit, 4 );
    tempExif += 4 ;
    //write FocalPlaneResolutionUnit
    FocalPlaneResolutionUnit[0] = exifFileInfo->FocalPlaneResolutionUnit;
    FocalPlaneResolutionUnitChar[0] = ( unsigned char ) ( FocalPlaneResolutionUnit[0] );
    FocalPlaneResolutionUnitChar[1] = ( unsigned char ) ( FocalPlaneResolutionUnit[0]>>8 );
    FocalPlaneResolutionUnitChar[2] = ( unsigned char ) ( FocalPlaneResolutionUnit[0]>>16 );
    FocalPlaneResolutionUnitChar[3] = ( unsigned char ) ( FocalPlaneResolutionUnit[0]>>24 );

    memcpy ( tempExif, FocalPlaneResolutionUnitChar, 4 );
    tempExif += 4 ;

    ///////////////ENTRY NO 19 :UserComments////////////////////////
    //write model tag
    memcpy ( tempExif, UserCommentstag, 2 );
    tempExif += 2 ;
    //write format
    memcpy ( tempExif, UserCommentsformat, 2 );
    tempExif += 2 ;
    //write no of component
    memcpy ( tempExif, NcomponentUserComments, 4 ); //sanjeev
    tempExif += 4 ;
    //write model
    //	strcpy(model,tpJEInfo->Model);
    memcpy ( UserComments,exifFileInfo->UserComments,150 );
    santemp= ( int ) ( offset );
    UserCommentsoffchar[0]= ( unsigned char ) santemp;
    UserCommentsoffchar[1]= ( unsigned char ) ( santemp>>8 );
    UserCommentsoffchar[2]= ( unsigned char ) ( santemp>>16 );
    UserCommentsoffchar[3]= ( unsigned char ) ( santemp>>24 );
    //write the User Comments offset into the bitstream
    memcpy ( tempExif, UserCommentsoffchar, 4 );
    tempExif += 4 ;
    memcpy ( startoftiff+santemp, UserComments, 128 );
    offset+=128;

    //////////////WRITE END OF ENTRY FLAG//////////////////
    memcpy ( tempExif, EndOfEntry, 4 );
    tempExif += 4 ;
    santemp= ( unsigned int ) ( offset );


    //////////////////////ENTRIES ARE OVER/////////////////////////////////
    ExifSize = ( santemp ) +8;
    ExifLen[1] = ( unsigned char ) ( ExifSize );
    ExifLen[0] = ( unsigned char ) ( ExifSize>>8 );

    if ( ExifSize > EXIF_FILE_SIZE + MAX_FILE_THUMB_SIZE - 2 || ExifSize < 0 )
    {
        printf( "makeExifFile  Invalid Exif size\r\n" );
        tempExif = NULL;
        *totalLen = 0;
        return -1;
    }

    tempExif = ExifInitialCount;
    memcpy ( tempExif, APP1Marker, 2 );
    tempExif += 2 ;
    memcpy ( tempExif, ExifLen, 2 );
    *totalLen = ExifSize + 2;
    //printf ( "makeExifFile  totalLen : %d\n", *totalLen );
    return 0;
}


/*
 *  填充Exif数据
 *  @param  exifFileInfo EXIF结构体
 *  @return 无
 */
static void makeExifParam(ExifFileInfo *exifFileInfo)
{
        strcpy ( exifFileInfo->Make,"Deep Guide" );
        strcpy ( exifFileInfo->Model,"YH-2MPA122-V1.0" );
        strcpy ( exifFileInfo->Version,"Greein_Camera_v1.0" );

        //strcpy ( exifFileInfo->DateTime,"2019.05.12 12:07" );  //修改时间

        //拍摄时间
        struct tm *ptr;
        time_t t;
        char nowtime[21];
        time(&t);
        ptr = localtime(&t);
        strftime(nowtime,sizeof(nowtime),"%Y-%m-%d %H:%M:%S",ptr); //时间格式化
        strcpy ( exifFileInfo->DateTimeOriginal,nowtime );


        strcpy ( exifFileInfo->CopyRight,"Greein@All Rights Reserved" );

        exifFileInfo->Width                                     = 960;
        exifFileInfo->Height                                    		= 840;
        exifFileInfo->Orientation                               = 1;
        exifFileInfo->ColorSpace                                = 1;
        exifFileInfo->Process                                   = 1;
        exifFileInfo->Flash                                             = 0;
        exifFileInfo->FocalLengthNum                    = 29;
        exifFileInfo->FocalLengthDen                    = 10;
        exifFileInfo->ExposureTimeNum                   = 10;
        exifFileInfo->ExposureTimeDen                   = ex_value;
        exifFileInfo->FNumberNum                                = 12;
        exifFileInfo->FNumberDen                                = 5;
        exifFileInfo->ApertureFNumber                   = 1;
        exifFileInfo->SubjectDistanceNum                = -5;
        exifFileInfo->SubjectDistanceDen                = -7;
        exifFileInfo->CCDWidth                                  = 1;
        exifFileInfo->ExposureBiasNum                   = -0;
        exifFileInfo->ExposureBiasDen                   = -2;
        exifFileInfo->WhiteBalance                              = 1;
        exifFileInfo->MeteringMode                              = 3;
        exifFileInfo->ExposureProgram                   = 1;
        exifFileInfo->ISOSpeedRatings[0]                = 1;
        exifFileInfo->ISOSpeedRatings[1]                = 2;
        exifFileInfo->FocalPlaneXResolutionNum  = 65;
        exifFileInfo->FocalPlaneXResolutionDen  = 66;
        exifFileInfo->FocalPlaneYResolutionNum  = 70;
        exifFileInfo->FocalPlaneYResolutionDen  = 71;
        exifFileInfo->FocalPlaneResolutionUnit  = 3;
        exifFileInfo->XResolutionNum                    = 48;
        exifFileInfo->XResolutionDen                    = 20;
        exifFileInfo->YResolutionNum                    = 48;
        exifFileInfo->YResolutionDen                    = 20;
        exifFileInfo->RUnit                                             = 2;
        exifFileInfo->BrightnessNum                             = 0;
        exifFileInfo->BrightnessDen                             = 1;

        strcpy(exifFileInfo->UserComments,"         Greein_Camera             ");
}

/*
 *  重构带有EXIF信息的JPEG图片
 *  @param  src_file   原始图片
 *  @param  dest_file  输出图片
 *  @param  buf   EXIF信息缓冲区
 *  @param  size  EXIF信息长度
 *  @return int  返回值，0为成功，-1为出错
 */
int makeNewSatJpgFromBuf( IN const UCHAR *src_file, IN const UCHAR *dest_file, IN UCHAR *buf, IN int size )
{
    int src_fd = open( src_file, O_RDONLY );
    if( -1 == src_fd ) {
            printf("++++++open file failed %s,%d\n", __FILE__, __LINE__);
            return -1;
    }

    int src_size = -1;
    struct stat _stat;
    int value = fstat( src_fd, &_stat );
    if( 0 == value ) {
            src_size = _stat.st_size;
            printf("++++++get file size %d\n", src_size);
    } else {
            printf("++++++get file size failed++++%s,%d\n", __FILE__, __LINE__);
            close(src_fd);
            return -1;
    }

    int dest_fd = open( dest_file, O_RDWR | O_CREAT );
    if( -1 == dest_fd ) {
            printf("++++++open file failed %s,%d\n", __FILE__, __LINE__);
            close(src_fd);
            return -1;
    }

    char *buf_dest = malloc( src_size );
    int read_size = read( src_fd, buf_dest, src_size );
            //printf("++++++read file read_size:%d  src_size:%d\n", read_size,src_size);
    close(src_fd);
    if( read_size != src_size ) {
            printf("++++++read file failed %s,%d\n", __FILE__, __LINE__);
            free(buf_dest);
            close(dest_fd);
            return -1;
    }
    int write_size = write( dest_fd, buf_dest, src_size );//copy src to dest jpg
            printf("++++++read file write_size:%d  src_size:%d\n", write_size,src_size);
    free(buf_dest);
    if( write_size != src_size ) {
            printf("++++++open file failed %s,%d\n", __FILE__, __LINE__);
            close(dest_fd);
            return -1;
    }
    lseek(dest_fd, 0, SEEK_SET);

    int tag_reserve_size = 500;
    int buf_out_size = size + tag_reserve_size;//will to write 500+_data_length
    char *buf_tail = malloc( src_size );//src

    unsigned char buf_data[4];
    read( dest_fd, buf_data, 2 );
    if( buf_data[0] != 0xFF || buf_data[1] != 0xD8 ) {
            printf("++++++error++++%s,%d\n", __FILE__, __LINE__);
            free(buf_tail);
            close(dest_fd);
            return -1;
    }
    while( 1 ) {
        read( dest_fd, buf_data, 4 );
        if( (buf_data[0] == 0xFF) && (buf_data[1]==0xDB) ) {
            lseek(dest_fd, -4, SEEK_CUR);
            int _tail_size = read( dest_fd, buf_tail, write_size );
            if( _tail_size > 0 ) {
                    lseek( dest_fd, -_tail_size, SEEK_CUR );
                    write( dest_fd, buf, size );//add
                    write( dest_fd, buf_tail, _tail_size );//src
                    break;
            }
        }
        else {
            unsigned int app_size = (buf_data[2]<<8) + buf_data[3];
            int offset = app_size - 2;
            if( -1 == lseek( dest_fd, offset, SEEK_CUR ) ) {
                printf("_____________erro");
                free(buf_tail);
                close(dest_fd);
                return -1;
            }
        }
    }

    free(buf_tail);
    close(dest_fd);
    return 0;
}

/*
 *  JPEG图片加入EXIF信息
 *  @param  cwb_flag  是否进行白平衡的标志，1已进行，0未进行
 *  @return int  返回值，0为成功，-1为出错
 */
int makeExif(int cwb_flag){

    ExifFileInfo    *ExifInfo;
    ExifInfo = ( ExifFileInfo* ) malloc ( sizeof(ExifFileInfo));
    memset(ExifInfo, 0x00, sizeof(ExifFileInfo));
        makeExifParam(ExifInfo);

    char			*ExifBuf;
        UINT                    ExifLen;
        UINT                    bufSize;
    bufSize = EXIF_FILE_SIZE;
    ExifBuf = ( char * ) malloc ( bufSize );
        memset ( ExifBuf, 0x20, bufSize );
    //printf("ExifBuf has obtain %d byte\n",bufSize);

    makeExifFile ( ExifBuf, &ExifLen, ExifInfo);


    //构造图片名
    UCHAR infile[100] = "/media/mmcblk1p1/pictures/src.jpg";

    struct tm *ptr;
    time_t t;
    UCHAR outfile[100];
    time(&t);
    ptr = localtime(&t);
    if(!cwb_flag){  //未进行白平衡矫正
        strftime(outfile,sizeof(outfile),"/media/mmcblk1p1/pictures/Greein_%Y%m%d_%H%M%S_ucwb",ptr);
        strcat(outfile,".jpg");
    }
    else{  //已进行白平衡矫正
        strftime(outfile,sizeof(outfile),"/media/mmcblk1p1/pictures/Greein_%Y%m%d_%H%M%S",ptr);
        strcat(outfile,".jpg");
    }

    makeNewSatJpgFromBuf( infile, outfile,ExifBuf, ExifLen);
    free(ExifInfo);
    free(ExifBuf);

    return 0;
}
