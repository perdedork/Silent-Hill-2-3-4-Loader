
/*
   ...

   Aug 19th:
   Added support for runlength encoding - changed some stuff around to make this
   possible.  Works well :)

   Sept 7th:
   Improved error trapping and recovery.

   Oct 22nd:
   Major source clearout & Can compress the image using S3_TC algorithm if the 
   driver supports it.
  
   Nov 10th:
   'Settled' version of the code - traps for nearly all errors - added a 
   LoadAndBind function for even lazier people :)
   TGA_NO_PASS was added in case you need to load an image and pass it yourself.
   Finally exorcised all the paletted texture code...
*/

 #include <windows.h>
 #include <gl\glu.h>
 #include <gl\glext.h>
 #include <gl\wglext.h>
// #include <mem.h>

#include "typedefs.h"
#include <stdio.h>
#include "tgaload.h"
#include "Renderer.h"

#ifndef __PRINT_ERRORS__
//extern bool debugMode;
#endif /*__PRINT_ERRORS__*/


#ifdef WINDOWS
/* Extension Management */
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC  glCompressedTexImage2DARB  = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC glGetCompressedTexImageARB = NULL;
#endif

/* Default support - lets be optimistic! */
bool tgaCompressedTexSupport = true;


void tgaGetExtensions ( void )
{  
#ifdef WINDOWS
   glCompressedTexImage2DARB  = ( PFNGLCOMPRESSEDTEXIMAGE2DARBPROC  ) 
                   wglGetProcAddress ( "glCompressedTexImage2DARB"  );
   glGetCompressedTexImageARB = ( PFNGLGETCOMPRESSEDTEXIMAGEARBPROC ) 
                   wglGetProcAddress ( "glGetCompressedTexImageARB" );
   
   if ( glCompressedTexImage2DARB == NULL || glGetCompressedTexImageARB == NULL )
      tgaCompressedTexSupport = false;
#endif
}


void tgaSetTexParams  ( unsigned int min_filter, unsigned int mag_filter, float anisotropy, unsigned int application )
{
	float fLargest;

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);

	if(anisotropy > fLargest)
		anisotropy=fLargest;

	glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, application );
}


unsigned char *tgaAllocMem ( tgaHeader_t info )
{
   unsigned char  *block;

   block = (unsigned char*) malloc ( info.bytes );

   if ( block == NULL )
      return 0;

   memset ( block, 0x00, info.bytes );

   return block;
}

void tgaPutPacketTuples ( image_t *p, unsigned char *temp_colour, int &current_byte )
{
   if ( p->info.components == 3 )
   {
      p->data[current_byte]   = temp_colour[2];
      p->data[current_byte+1] = temp_colour[1];
      p->data[current_byte+2] = temp_colour[0];
      current_byte += 3;
   }

   if ( p->info.components == 4 )    // Because its BGR(A) not (A)BGR :(
   {
      p->data[current_byte]   = temp_colour[2];
      p->data[current_byte+1] = temp_colour[1];
      p->data[current_byte+2] = temp_colour[0];
      p->data[current_byte+3] = temp_colour[3];
      current_byte += 4;
   }
}


void tgaGetAPacket ( int &current_byte, image_t *p, FILE *file )
{
   unsigned char  packet_header;
   int            run_length;
   unsigned char  temp_colour[4] = { 0x00, 0x00, 0x00, 0x00 };

   fread        ( &packet_header, ( sizeof ( unsigned char )), 1, file );
   run_length = ( packet_header&0x7F ) + 1;

   if ( packet_header&0x80 )  // RLE packet
   {
      fread ( temp_colour, ( sizeof ( unsigned char )* p->info.components ), 1, file );

      if ( p->info.components == 1 )  // Special optimised case :)
      {
         memset ( p->data + current_byte, temp_colour[0], run_length );
         current_byte += run_length;
      } else
      for ( int i = 0; i < run_length; i++ )
         tgaPutPacketTuples ( p, temp_colour, current_byte );
   }

   if ( !( packet_header&0x80 ))  // RAW packet
   {
      for ( int i = 0; i < run_length; i++ )
      {
         fread ( temp_colour, ( sizeof ( unsigned char )* p->info.components ), 1, file );

         if ( p->info.components == 1 )
         {
            memset ( p->data + current_byte, temp_colour[0], run_length );
            current_byte += run_length;
         } else
            tgaPutPacketTuples ( p, temp_colour, current_byte );
      }
   }
} 


void tgaGetPackets ( image_t *p, FILE *file )
{
  int current_byte = 0;

  while ( current_byte < p->info.bytes )
    tgaGetAPacket ( current_byte, p, file );
}


void tgaGetImageData ( image_t *p, FILE *file )
{
//   unsigned char  *data;
   unsigned char  temp;

   p->data = tgaAllocMem ( p->info );
if(p->data==NULL)
MessageBox(NULL,"Data is NULL in TGA Load!!!!!!","CRITICAL ERROR!!!",MB_OK);
   /* Easy unRLE image */
   if ( p->info.image_type == 1 || p->info.image_type == 2 || p->info.image_type == 3 )
   {   
      fread ( p->data, sizeof (unsigned char), p->info.bytes, file );

      /* Image is stored as BGR(A), make it RGB(A)     */
      for (int i = 0; i < p->info.bytes; i += p->info.components )
      {
         temp = p->data[i];
         p->data[i] = p->data[i + 2];
         p->data[i + 2] = temp;
      }
   }
   
   /* RLE compressed image */
   if ( p->info.image_type == 9 || p->info.image_type == 10 )
      tgaGetPackets ( p, file );
}


void tgaUploadImage ( image_t *p, tgaFLAG mode )
{
   /*  Determine TGA_LOWQUALITY  internal format
       This directs OpenGL to upload the textures at half the bit
       precision - saving memory
    */
   GLenum internal_format = p->info.tgaColourType;

        if ( mode&TGA_LOW_QUALITY )
   {
      switch ( p->info.tgaColourType )
      {
         case GL_RGB       : internal_format = GL_RGB4; break;
         case GL_RGBA      : internal_format = GL_RGBA4; break; 
         case GL_LUMINANCE : internal_format = GL_LUMINANCE4; break;
         case GL_ALPHA     : internal_format = GL_ALPHA4; break;          
      }
   }

   /*  Let OpenGL decide what the best compressed format is each case. */   
   if ( mode&TGA_COMPRESS && tgaCompressedTexSupport )
   {            
      switch ( p->info.tgaColourType )
      {
         case GL_RGB       : internal_format = GL_COMPRESSED_RGB_ARB; break;
         case GL_RGBA      : internal_format = GL_COMPRESSED_RGBA_ARB; break; 
         case GL_LUMINANCE : internal_format = GL_COMPRESSED_LUMINANCE_ARB; break; 
         case GL_ALPHA     : internal_format = GL_COMPRESSED_ALPHA_ARB; break;          
      }
   }                      
                        
   /*  Pass OpenGL Texture Image */
   if ( !( mode&TGA_NO_MIPMAPS ))
      gluBuild2DMipmaps ( GL_TEXTURE_2D, internal_format, p->info.width,
                          p->info.height, p->info.tgaColourType, GL_UNSIGNED_BYTE, p->data );
   else
      glTexImage2D ( GL_TEXTURE_2D, 0, internal_format, p->info.width,
                     p->info.height, 0, p->info.tgaColourType, GL_UNSIGNED_BYTE, p->data );  
}


void tgaFree ( image_t *p )
{
   if ( p->data != NULL )
      free ( p->data );
}


void tgaChecker ( image_t *p )
{
  unsigned char TGA_CHECKER[16384];
  unsigned char *pointer;

  // 8bit image
  p->info.image_type = 3;

  p->info.width  = 128;
  p->info.height = 128;

  p->info.pixel_depth = 8;

  // Set some stats
  p->info.components = 1;
  p->info.bytes      = p->info.width * p->info.height * p->info.components;

  pointer = TGA_CHECKER;

  for ( int j = 0; j < 128; j++ )
  {
    for ( int i = 0; i < 128; i++ )
    {
      if ((i ^ j) & 0x10 )
        pointer[0] = 0x00;
      else
        pointer[0] = 0xff;
      pointer ++;
    }
  }

  p->data = TGA_CHECKER;

  glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE4, p->info.width,
                 p->info.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p->data );

/*  Should we free?  I dunno.  The scope of TGA_CHECKER _should_ be local, so it
    probably gets destroyed automatically when the function completes... */
//  tgaFree ( p );
}


void tgaError ( char *error_string, char *file_name, FILE *file, image_t *p )
{
	char messageString[200];
	sprintf  (messageString, "TGA LOAD ERROR::%s - %s\n", error_string, file_name );
//	MessageBox(NULL,messageString,"TGA LOAD ERROR!!",MB_OK);
#ifndef __PRINT_ERRORS__
//	if( debugMode )
#endif /*__PRINT_ERRORS__*/
	LogFile(ERROR_LOG,messageString);
	tgaFree ( p );

//   fclose ( file );

   //tgaChecker ( p );
}


void tgaDebugHeader(tgaHeader_t *info)
{
	LogFile(ERROR_LOG,"\nTGA Header\n-------------------------");
	LogFile(ERROR_LOG,"id_length: %d",info->id_length);
	LogFile(ERROR_LOG,"colour_map_type: %d",info->colour_map_type);
	LogFile(ERROR_LOG,"image_type: %d",info->image_type);
	LogFile(ERROR_LOG,"x_origin: %d",info->x_origin);
	LogFile(ERROR_LOG,"y_origin: %d",info->y_origin);
	LogFile(ERROR_LOG,"width: %d",info->width);
	LogFile(ERROR_LOG,"height: %d",info->height);
	LogFile(ERROR_LOG,"pixel_depth: %d",info->pixel_depth);
	LogFile(ERROR_LOG,"components: %d",info->components);
	LogFile(ERROR_LOG,"image_descriptor: %d",info->image_descriptor);
	LogFile(ERROR_LOG,"bytes: %d",info->bytes);
	LogFile(ERROR_LOG,"colour_map_first_entry: %d",info->colour_map_first_entry);
	LogFile(ERROR_LOG,"colour_map_length: %d",info->colour_map_length);
	LogFile(ERROR_LOG,"colour_map_entry_size: %d\n",info->colour_map_entry_size);
}

void tgaGetImageHeader ( FILE *file, tgaHeader_t *info )
{
   /*   Stupid byte alignment means that we have to fread each field
        individually.  I tried splitting tgaHeader into 3 structures, no matter
        how you arrange them, colour_map_entry_size comes out as 2 bytes instead
        1 as it should be.  Grrr.  Gotta love optimising compilers - theres a pragma
        for Borland, but I dunno the number for MSVC or GCC :(
    */
   fread ( &info->id_length,       ( sizeof (unsigned char )), 1, file );
   fread ( &info->colour_map_type, ( sizeof (unsigned char )), 1, file );
   fread ( &info->image_type,      ( sizeof (unsigned char )), 1, file );

   fread ( &info->colour_map_first_entry, ( sizeof (short int )), 1, file );
   fread ( &info->colour_map_length     , ( sizeof (short int )), 1, file );
   fread ( &info->colour_map_entry_size , ( sizeof (unsigned char )), 1, file );

   fread ( &info->x_origin , ( sizeof (short int )), 1, file );
   fread ( &info->y_origin , ( sizeof (short int )), 1, file );
   fread ( &info->width,     ( sizeof (short int )), 1, file );
   fread ( &info->height,    ( sizeof (short int )), 1, file );

   fread ( &info->pixel_depth,     ( sizeof (unsigned char )), 1, file );
   fread ( &info->image_descriptor,( sizeof (unsigned char )), 1, file );

   // Set some stats
   info->components = info->pixel_depth / 8;
   info->bytes      = info->width * info->height * info->components;
}

int tgaLoadTheImage ( char *file_name, image_t *p, tgaFLAG mode )
{
   FILE   *file;

   tgaGetExtensions ( );

   if (( file = fopen ( file_name, "rb" )) == NULL )
   {
      tgaError ( "File not found", file_name, file, p );
      return 0;
   }

   tgaGetImageHeader ( file, &p->info );

   switch ( p->info.image_type )
   {
      case 1 :
         tgaError ( "8-bit colour no longer supported", file_name, file, p );
         break;

      case 2 :
         if ( p->info.pixel_depth == 24 )
            p->info.tgaColourType = GL_RGB;
         else if ( p->info.pixel_depth == 32 )
            p->info.tgaColourType = GL_RGBA;
		 else
		 {
			 tgaError ( "Unsupported RGB format", file_name, file, p );
			 return 0;
		 }
         break;

      case 3 :
         if ( mode&TGA_LUMINANCE )
            p->info.tgaColourType = GL_LUMINANCE;
         else if ( mode&TGA_ALPHA )
            p->info.tgaColourType = GL_ALPHA;
        else
		{
			tgaError ( "Must be LUMINANCE or ALPHA greyscale", file_name, file, p );
			return 0;
		}
         break;

      case 9 :
		  {
			  tgaError ( "8-bit colour no longer supported", file_name, file, p );
			  return 0;
		  }
		  break;
	  
      case 10 :
         if ( p->info.pixel_depth == 24 )
            p->info.tgaColourType = GL_RGB;
         else if ( p->info.pixel_depth == 32 )
            p->info.tgaColourType = GL_RGBA;
         else
		 {
			 tgaError ( "Unsupported compressed RGB format", file_name, file, p );
			 return 0;
		 }
         break;
	  default:char mess[228];
		  sprintf(mess,"Image type:%d  pixel depth:%d  color type:%s",p->info.image_type,p->info.pixel_depth,
			  (p->info.tgaColourType==GL_RGBA)?"RGBA":((p->info.tgaColourType==GL_RGB)?"RGB":"Other"));
		  tgaError(mess,file_name,file,p);
		  return 0;
		  break;
   }

   tgaGetImageData ( p, file );

   fclose  ( file );
   return 1;
}


int tgaSave( char *filename, image_t *p, bool swapRB, bool debugVals )
{
	FILE *file;
	tgaHeader_t *info = &p->info;
	long k;

	if((file = fopen(filename,"wb+"))==NULL)
	{
		LogFile(ERROR_LOG,"ERROR tgaSave(): Unable to open file '%s' for writing",filename);
		return 0;
	}

	k = fwrite ( &info->id_length,       ( sizeof (unsigned char )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for id_length",k);
	k = fwrite ( &info->colour_map_type, ( sizeof (unsigned char )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for colour_map_type",k);
	k = fwrite ( &info->image_type,      ( sizeof (unsigned char )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for image_type",k);

	k = fwrite ( &info->colour_map_first_entry, ( sizeof (short int )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for colour_map_first_entry",k);
	k = fwrite ( &info->colour_map_length     , ( sizeof (short int )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for colour_map_length",k);
	k = fwrite ( &info->colour_map_entry_size , ( sizeof (unsigned char )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for colour_map_entry_size",k);

	k = fwrite ( &info->x_origin , ( sizeof (short int )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for x origin",k);
	k = fwrite ( &info->y_origin , ( sizeof (short int )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for y origin",k);
	k = fwrite ( &info->width,     ( sizeof (short int )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for width",k);
	k = fwrite ( &info->height,    ( sizeof (short int )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for height",k);

	k = fwrite ( &info->pixel_depth,     ( sizeof (unsigned char )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for pixel_depth",k);
	k = fwrite ( &info->image_descriptor,( sizeof (unsigned char )), 1, file );
	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Wrote %d bytes for image descriptior",k);

	if( debugVals ) LogFile(ERROR_LOG,"TEST tgaSave(): Size of image %d",info->width*info->height*info->components);

	if( swapRB )
	{
		for( k = 0; k < info->width * info->height * info->components; k+=info->components )
			swap( p->data[ k ], p->data[ k + 2 ] );
	}

	fwrite(p->data,(sizeof(unsigned char )),info->width*info->height*info->components, file );
	fclose(file);
	return 1;
}


void tgaLoad ( char *file_name, image_t *p, tgaFLAG mode )
{
   if(tgaLoadTheImage ( file_name, p, mode )==0)
	   return;

   if  ( !( mode&TGA_NO_PASS ))
      tgaUploadImage  ( p, mode );

   if ( mode&TGA_FREE )
     tgaFree ( p );

}

GLuint tgaLoadAndBind ( const char *file_name, tgaFLAG mode, int *flags )
{
   GLuint   texture_id;
   //FILE     *file;
   image_t  p;
p.data=NULL;
   if(tgaLoadTheImage ( (char*)file_name, &p, mode )==0)
	   return 0;
glEnable(GL_TEXTURE_2D);
   glGenTextures ( 1, &texture_id );
   glBindTexture ( GL_TEXTURE_2D, texture_id );

   tgaUploadImage  ( &p, mode );
   glDisable(GL_TEXTURE_2D);
   if( flags !=NULL )
   {
	   flags[0]=0;
	   flags[0]=(p.info.components==4)?TEXM_32_BPP:((p.info.components==3)?TEXM_24_BPP:TEXM_16_BPP);
   }
   tgaFree       ( &p );

   return texture_id;
}

int tgaLoadAndBind2( const char *file_name, tgaFLAG mode,GLuint texture_id )
{

   image_t  p;
p.data=NULL;
   if(tgaLoadTheImage ( (char*)file_name, &p, mode )==0)
	   return 0;

   glBindTexture ( GL_TEXTURE_2D, texture_id );
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   tgaUploadImage  ( &p, mode );

   tgaFree       ( &p );

   return 1;
}


int tgaLoadAndBind3( const char *file_name, tgaFLAG mode,GLuint texture_id )
{

   image_t  p;
   p.data=NULL;
   int x,y;
   if(tgaLoadTheImage ( (char*)file_name, &p, mode )==0)
	   return 0;
    char mess[228];
		  sprintf(mess,"File: %s  Image type:%d  pixel depth:%d  color type:%s\n",file_name,p.info.image_type,p.info.pixel_depth,
			  (p.info.tgaColourType==GL_RGBA)?"RGBA":((p.info.tgaColourType==GL_RGB)?"RGB":"Other"));
	//	  LogFile(ERROR_LOG,mess);
   if(p.info.pixel_depth == 32)
   {
	   unsigned char *tData = new unsigned char[p.info.width * p.info.height * 3];
//	   unsigned char *temp;
	   for(y=0; y < p.info.height; y++)
	   {
		   for(x=0; x < p.info.width; x++)
		   {
			   tData[y * p.info.width * 3 + x * 3] = p.data[y * p.info.width * 4 + x * 4];
			   tData[y * p.info.width * 3 + x * 3+1] = p.data[y * p.info.width * 4 + x * 4+1];
			   tData[y * p.info.width * 3 + x * 3+2] = p.data[y * p.info.width * 4 + x * 4+2];
			   //p.data[y * p.info.width * size + x * size + 3]= 255;
		   }
	   }
	   delete []p.data;
	   p.data= tData;
	   p.info.tgaColourType=GL_RGB;
   }

   glBindTexture ( GL_TEXTURE_2D, texture_id );
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   tgaUploadImage  ( &p, mode );

   tgaFree       ( &p );

   return 1;
}



int tgaLoadSeperateChannels(const char *redTex, const char *greenTex, const char *blueTex, const char *alphaTex, tgaFLAG mode, GLuint texture_id )
{
	image_t pR, pB, pG, pA;

	pR.data = pG.data = pB.data = pA.data = NULL;

	int x, y;

	int d = 3;

	if( redTex != NULL )
	{
		if( tgaLoadTheImage ( (char*)redTex, &pR, mode )==0)
			return 0;
	}
	else
	{
		LogFile(ERROR_LOG, " ERROR: tgaLoadSeperateChannels - No Red Texture Defined.  Must be defined in order to use function correctly!");
		return 0;
	}

	if( greenTex != NULL )
	{
		if( tgaLoadTheImage ( (char*)greenTex, &pG, mode )==0)
			return 0;
	}

	if( blueTex != NULL )
	{
		if( tgaLoadTheImage ( (char*)blueTex, &pB, mode )==0)
			return 0;
	}

	if( alphaTex != NULL )
	{
		d = 4;
		if( tgaLoadTheImage ( (char*)alphaTex, &pA, mode )==0)
			return 0;
	}

	unsigned char *tData = new unsigned char[pR.info.width * pR.info.height * 4];

LogFile(ERROR_LOG,"Here 1");

	for(y=0; y < pR.info.height; y++)
	{
		for(x=0; x < pR.info.width; x++)
		{
			/*if( mode & TGA_FILL )  // This option will fill in missing color channels with the prior ones.  See the header 'tgaload.h' for a complete desc
			{
				//-------------[ CHANNEL RED ]---------------/
				if( redTex != NULL )
					tData[y * pR.info.width * 3 + x * 3] = pR.data[y * pR.info.width * 4 + x * 4];
				else if( greenTex != NULL )
					tData[y * pR.info.width * 3 + x * 3] = pG.data[y * pR.info.width * 4 + x * 4];
				else if( blueTex != NULL )
					tData[y * pR.info.width * 3 + x * 3] = pB.data[y * pR.info.width * 4 + x * 4];
				else if( alphaTex != NULL )
					tData[y * pR.info.width * 3 + x * 3] = pA.data[y * pR.info.width * 4 + x * 4];
				else tData[y * pR.info.width * 3 + x * 3] = 0;

				//-------------[ CHANNEL GREEN ]---------------/
				if( greenTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+1] = pG.data[y * pR.info.width * 4 + x * 4+1];
				else if( redTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+1] = pR.data[y * pR.info.width * 4 + x * 4+1];
				else if( blueTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+1] = pB.data[y * pR.info.width * 4 + x * 4+1];
				else if( alphaTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+1] = pA.data[y * pR.info.width * 4 + x * 4+1];
				else tData[y * pR.info.width * 3 + x * 3] = 0;

				//-------------[ CHANNEL BLUE ]---------------/
				if( blueTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+2] = pB.data[y * pR.info.width * 4 + x * 4+2];
				else if( greenTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+2] = pG.data[y * pR.info.width * 4 + x * 4+2];
				else if( redTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+2] = pR.data[y * pR.info.width * 4 + x * 4+2];
				else if( alphaTex != NULL )
					tData[y * pR.info.width * 3 + x * 3+2] = pA.data[y * pR.info.width * 4 + x * 4+2];
				else tData[y * pR.info.width * 3 + x * 3] = 0;

				//-------------[ CHANNEL ALPHA ]---------------/
				if( alphaTex != NULL && pA.info.components==4)
					tData[y * pR.info.width * 4 + x * 3+3] = pA.data[y * pR.info.width * 4 + x * 4+3];
				else if( blueTex != NULL && pB.info.components==4)
					tData[y * pR.info.width * 4 + x * 3+3] = pB.data[y * pR.info.width * 4 + x * 4+3];
				else if( greenTex != NULL && pR.info.components==4)
					tData[y * pR.info.width * 4 + x * 3+3] = pG.data[y * pR.info.width * 4 + x * 4+3];
				else if( redTex != NULL && pR.info.components==4)
					tData[y * pR.info.width * 4 + x * 3+3] = pR.data[y * pR.info.width * 4 + x * 4+3];
				else tData[y * pR.info.width * 4 + x * 3] = 0;
			}
			else */  //Default option.  If a color channel is missing, set it to '0'
			
				//-------------[ CHANNEL RED ]---------------/
				if( redTex != NULL )
					tData[y * pR.info.width * d + x * d] = pR.data[y * pR.info.width * pR.info.components + x * pR.info.components];
				else tData[y * pR.info.width * d + x * d] = 0;

				//-------------[ CHANNEL GREEN ]---------------/
				if( greenTex != NULL )
					tData[y * pR.info.width * d + x * d+1] = pG.data[y * pR.info.width * pG.info.components + x * pG.info.components+1];
				else tData[y * pR.info.width * d + x * d+1] = 0;

				//-------------[ CHANNEL BLUE ]---------------/
				if( blueTex != NULL )
					tData[y * pR.info.width * d + x * d+2] = pB.data[y * pR.info.width * pB.info.components + x * pB.info.components+2];
				else tData[y * pR.info.width * d + x * d+2] = 0;

				//-------------[ CHANNEL ALPHA ]---------------/
				if( alphaTex != NULL )
					tData[y * pR.info.width * d + x * d+ d-1] = pA.data[y * pR.info.width * pA.info.components + x * pA.info.components+pA.info.components-1];
				else tData[y * pR.info.width * d + x * d+ d-1 ] = 0;
			
		}
	}

	LogFile(ERROR_LOG,"Here 2");
	if( redTex != NULL )
		delete [] pR.data;
	if( greenTex != NULL )
		delete [] pG.data;
	if( blueTex != NULL )
		delete [] pB.data;
	if( alphaTex != NULL )
		delete [] pA.data;

char mess[228];
		  sprintf(mess,"File: %s  Image type:%d  pixel depth:%d  color type:%s\n",redTex,pR.info.image_type,pR.info.pixel_depth,
			  (pR.info.tgaColourType==GL_RGBA)?"RGBA":((pR.info.tgaColourType==GL_RGB)?"RGB":"Other"));
		  LogFile(ERROR_LOG,mess);

	pR.data= tData;
	pR.info.tgaColourType=GL_RGB;
 
	if(blueTex != NULL )
	{
	 sprintf(mess,"File: %s  Image type:%d  pixel depth:%d  color type:%s\n",blueTex,pB.info.image_type,pB.info.pixel_depth,
			  (pB.info.tgaColourType==GL_RGBA)?"RGBA":((pB.info.tgaColourType==GL_RGB)?"RGB":"Other"));
		  LogFile(ERROR_LOG,mess);
	}
	if(greenTex != NULL )
	{
	sprintf(mess,"File: %s  Image type:%d  pixel depth:%d  color type:%s\n",greenTex,pG.info.image_type,pG.info.pixel_depth,
			  (pG.info.tgaColourType==GL_RGBA)?"RGBA":((pG.info.tgaColourType==GL_RGB)?"RGB":"Other"));
		  LogFile(ERROR_LOG,mess);
	}

   glBindTexture ( GL_TEXTURE_2D, texture_id );
 //  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   tgaUploadImage  ( &pR, mode );

   tgaFree       ( &pR );

   return 1;
}