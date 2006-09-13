/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2006-12-09
 * Description : RAW file util methods using dcraw
 *
 * Copyright 2006 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// C++ includes.

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>

// C Ansi includes.

extern "C"
{
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <jpeglib.h>
#include <tiffio.h>
#include <tiffvers.h>
#include <png.h>
}

// Qt Includes.

#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>

// KIPI include files

#include <libkipi/version.h>

// Local includes.

#include "rawfiles.h"
#include "dcrawutils.h"

namespace KIPIRawConverterPlugin
{

bool DcrawUtils::loadDcrawPreview(QImage& image, const QString& path)
{
    FILE       *f;
    QByteArray  imgData;
    const int   MAX_IPC_SIZE = (1024*32);
    char        buffer[MAX_IPC_SIZE];
    QFile       file;
    Q_LONG      len;
    QCString    command;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(kipi_raw_file_extentions);

    if (!fileInfo.exists() || !rawFilesExt.upper().contains( fileInfo.extension().upper() ))
        return false;

    // Try to extract embedded thumbnail using dcraw with options:
    // -c : write to stdout
    // -e : Extract the camera-generated thumbnail, not the raw image (JPEG or a PPM file).
    // Note : this code require at least dcraw version 8.x

    command  = "dcraw -c -e ";
    command += QFile::encodeName( KProcess::quote( path ) );
    kdDebug( 51000 ) << "Running dcraw command " << command << endl;

    f = popen( command.data(), "r" );

    if ( !f )
        return false;

    file.open( IO_ReadOnly,  f );

    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            file.close();
            return false;
        }
        else
        {
            int oldSize = imgData.size();
            imgData.resize( imgData.size() + len );
            memcpy(imgData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );

    if ( !imgData.isEmpty() )
    {
        if (image.loadFromData( imgData ))
        {
            kdDebug() << "Using embedded RAW preview extraction" << endl;
            return true;
        }
    }

    // In second, try to use simple RAW extraction method in 8 bits ppm output.
    // -c : write to stdout
    // -h : Half-size color image (3x faster than -q)
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible

    command  = "dcraw -c -h -w -a ";
    command += QFile::encodeName( KProcess::quote( path ) );
    kdDebug( 51000 ) << "Running dcraw command " << command << endl;

    f = popen( command.data(), "r" );

    if ( !f )
        return false;

    file.open( IO_ReadOnly,  f );

    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            file.close();
            return false;
        }
        else
        {
            int oldSize = imgData.size();
            imgData.resize( imgData.size() + len );
            memcpy(imgData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );

    if ( !imgData.isEmpty() )
    {
        if (image.loadFromData( imgData ))
        {
            kdDebug() << "Using reduced RAW picture extraction" << endl;
            return true;
        }
    }

    return false;
}

bool DcrawUtils::rawFileIdentify(QString& identify, const QString& path)
{
    FILE       *f;
    QByteArray  txtData;
    const int   MAX_IPC_SIZE = (1024*32);
    char        buffer[MAX_IPC_SIZE];
    QFile       file;
    Q_LONG      len;
    QCString    command;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(kipi_raw_file_extentions);

    if (!fileInfo.exists())
    {
        identify = i18n("Cannot open RAW file");
        return false;
    }

    if (!rawFilesExt.upper().contains( fileInfo.extension().upper() ))
    {
        identify = i18n("Not a RAW file");
        return false;
    }

    // Try to get camera maker/model using dcraw with options:
    // -c : write to stdout
    // -i : identify files without decoding them.

    command  = "dcraw -c -i ";
    command += QFile::encodeName( KProcess::quote( path ) );
    kdDebug( 51000 ) << "Running dcraw command " << command << endl;

    f = popen( command.data(), "r" );

    if ( !f )
    {
        identify = i18n("Cannot start dcraw instance");
        return false;
    }

    file.open( IO_ReadOnly,  f );

    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            identify = i18n("Cannot identify RAW file");
            return false;
        }
        else
        {
            int oldSize = txtData.size();
            txtData.resize( txtData.size() + len );
            memcpy(txtData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );
    identify = QString(txtData);

    if ( identify.isEmpty() )
    {
        identify = i18n("Cannot identify RAW file");
        return false;
    }

    identify.remove(path);
    identify.remove(" is a ");
    identify.remove(" image.");

    return true;
}

bool DcrawUtils::decodeHalfRAWImage(const QString& filePath, QString& destPath,
                                    RawDecodingSettings rawDecodingSettings)
{
    rawDecodingSettings.halfSizeColorImage = true;
    rawDecodingSettings.outputFileFormat   = RawDecodingSettings::PPM;
    return(decodeRAWImage(filePath, destPath, rawDecodingSettings));
}

bool DcrawUtils::decodeRAWImage(const QString& filePath, QString& destPath,
                                RawDecodingSettings rawDecodingSettings)
{
    int  width, height, rgbmax;
    char nl;
    QCString command;

    // run dcraw with options:
    // -c : write to stdout
    // -h : Output a half-size color image. 
    // -w : Use camera white balance, if possible  
    // -a : Use automatic white balance
    // -f : Interpolate RGB as four colors. This blurs the image a little, but it eliminates false 2x2 mesh patterns.
    // -s : Use secondary pixels (Fuji Super CCD SR only)
    // -H : Unclip Highlight color level.
    // -b : Change the output brightness.
    // -q : Use simple bilinear interpolation for quick results
    // -B : Use bilateral filter to smooth noise while preserving edges.

    command = "dcraw";
    command += " -c ";

    if (rawDecodingSettings.halfSizeColorImage)
        command += "-h ";
    
    if (rawDecodingSettings.cameraColorBalance)
        command += "-w ";
    
    if (rawDecodingSettings.automaticColorBalance)
        command += "-a ";

    if (rawDecodingSettings.RGBInterpolate4Colors)
        command += "-f ";

    if (rawDecodingSettings.SuperCCDsecondarySensor)
        command += "-s ";

    command += "-H ";
    QCString unclipLevel;
    command += unclipLevel.setNum(rawDecodingSettings.unclipColors);
    command += " ";

    command += "-b ";
    QCString brightness;
    command += brightness.setNum(rawDecodingSettings.brightness);
    command += " ";

    command += "-q ";
    QCString rawQuality;
    command += rawQuality.setNum(rawDecodingSettings.RAWQuality);
    command += " ";

    if (rawDecodingSettings.enableNoiseReduction)
    {
        QCString NRSigmaDomain, NRSigmaRange;
        command += "-B ";
        command += NRSigmaDomain.setNum(rawDecodingSettings.NRSigmaDomain);
        command += " ";
        command += NRSigmaRange.setNum(rawDecodingSettings.NRSigmaRange);
        command += " ";
    }
    
    command += QFile::encodeName( KProcess::quote( filePath ) );

    kdDebug() << "Running dcraw command : " << command << endl;

    FILE* f = popen( command.data(), "r" );

    if ( !f )
    {
        kdDebug( 51000 ) << "dcraw program unavailable." << endl;
        return false;
    }

    if (fscanf (f, "P6 %d %d %d%c", &width, &height, &rgbmax, &nl) != 4) 
    {
        kdDebug( 51000 ) << "Not a raw digital camera image." << endl;
        pclose (f);
        return false;
    }

    // -------------------------------------------------------------------
    // Get image data

    uchar *imgData = new uchar[width*height*3];
    uchar *dst     = imgData;
    uchar  src[3];
    
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            fread (src, 3 *sizeof(uchar), 1, f);

            // No need to adapt RGB components accordinly with rgbmax value because dcraw
            // always return rgbmax to 255 in 8 bits/color/pixels.
            memcpy(dst, src, 3);    // BGR color components
            dst   += 3;
        }
    }
    
    pclose( f );

    QString soft = QString("Kipi Raw Converter v.%1").arg(kipi_version);
    QFileInfo fi(filePath);
    destPath = fi.dirPath(true) + QString("/") + ".kipi-rawconverter-tmp-" 
                                + QString::number(::time(0));

    switch(rawDecodingSettings.outputFileFormat)
    {
        case RawDecodingSettings::JPEG:
        {
            FILE* f = 0;
            f = fopen(QFile::encodeName(destPath), "wb");
    
            if (!f) 
            {
                kdDebug( 51000 ) << "Failed to open JPEG file for writing"
                                 << endl;
                delete [] imgData;     
                return false;
            }
    
            struct jpeg_compress_struct cinfo;
            struct jpeg_error_mgr       jerr;
    
            int      row_stride;
            JSAMPROW row_pointer[1];
    
            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_compress(&cinfo);
            jpeg_stdio_dest(&cinfo, f);
            cinfo.image_width      = width;
            cinfo.image_height     = height;
            cinfo.input_components = 3;
            cinfo.in_color_space   = JCS_RGB;
            jpeg_set_defaults(&cinfo);
            // B.K.O #130996: set horizontal and vertical Subsampling factor 
            // to 1 for a best quality of color picture compression. 
            cinfo.comp_info[0].h_samp_factor = 1;
            cinfo.comp_info[0].v_samp_factor = 1; 
            jpeg_set_quality(&cinfo, 100, true);
            jpeg_start_compress(&cinfo, true);
            row_stride = cinfo.image_width * 3;
            
            while (cinfo.next_scanline < cinfo.image_height)
            {
                row_pointer[0] = imgData + (cinfo.next_scanline * row_stride);
                jpeg_write_scanlines(&cinfo, row_pointer, 1);
            }
            
            jpeg_finish_compress(&cinfo);
            fclose(f);
            break;
        }
        case RawDecodingSettings::PNG:
        {
            FILE* f = 0;
            f = fopen(QFile::encodeName(destPath), "wb");
    
            if (!f) 
            {
                kdDebug( 51000 ) << "Failed to open PNG file for writing"
                                 << endl;
                delete [] imgData;     
                return false;
            }
    
            png_color_8 sig_bit;
            png_bytep   row_ptr;
            png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            png_infop info_ptr  = png_create_info_struct(png_ptr);
            png_init_io(png_ptr, f);
            png_set_IHDR(png_ptr, info_ptr, width, height, 8, 
                        PNG_COLOR_TYPE_RGB,        PNG_INTERLACE_NONE, 
                        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            sig_bit.red   = 8;
            sig_bit.green = 8;
            sig_bit.blue  = 8;
            sig_bit.alpha = 8;
            png_set_sBIT(png_ptr, info_ptr, &sig_bit);
            png_set_compression_level(png_ptr, 9);

            QString libpngver(PNG_HEADER_VERSION_STRING);
            libpngver.replace('\n', ' ');
            soft.append(QString(" (%1)").arg(libpngver));
            png_text text;
            text.key  = "Software";
            text.text = (char *)soft.ascii();
            text.compression = PNG_TEXT_COMPRESSION_zTXt;
            png_set_text(png_ptr, info_ptr, &(text), 1);

            png_write_info(png_ptr, info_ptr);
            png_set_shift(png_ptr, &sig_bit);
            png_set_packing(png_ptr);
            unsigned char* ptr  = imgData;
    
            for (int y = 0; y < height; y++)
            {
                row_ptr = (png_bytep) ptr;
                png_write_rows(png_ptr, &row_ptr, 1);
                ptr += (width * 3);
            }
    
            png_write_end(png_ptr, info_ptr);
            png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
            png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
            fclose(f);
            break;
        }
        case RawDecodingSettings::TIFF:
        {
            TIFF               *tif=0;
            unsigned char      *data;
            int                 y;
            int                 w;
            
            tif = TIFFOpen(QFile::encodeName(destPath), "wb");
    
            if (!tif) 
            {
                kdDebug( 51000 ) << "Failed to open TIFF file for writing"
                                 << endl;
                delete [] imgData;     
                return false;
            }
    
            TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,      width);
            TIFFSetField(tif, TIFFTAG_IMAGELENGTH,     height);
            TIFFSetField(tif, TIFFTAG_ORIENTATION,     ORIENTATION_TOPLEFT);
            TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
            TIFFSetField(tif, TIFFTAG_PLANARCONFIG,    PLANARCONFIG_CONTIG);
            TIFFSetField(tif, TIFFTAG_COMPRESSION,     COMPRESSION_ADOBE_DEFLATE);
            TIFFSetField(tif, TIFFTAG_ZIPQUALITY,      9);
            // NOTE : this tag values aren't defined in libtiff 3.6.1. '2' is PREDICTOR_HORIZONTAL.
            //        Use horizontal differencing for images which are
            //        likely to be continuous tone. The TIFF spec says that this
            //        usually leads to better compression.
            //        See this url for more details:
            //        http://www.awaresystems.be/imaging/tiff/tifftags/predictor.html
            TIFFSetField(tif, TIFFTAG_PREDICTOR,       2); 
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
            TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB);
            w = TIFFScanlineSize(tif);
            TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,    TIFFDefaultStripSize(tif, 0));

            QString libtiffver(TIFFLIB_VERSION_STR);
            libtiffver.replace('\n', ' ');
            soft.append(QString(" ( %1 )").arg(libtiffver));
            TIFFSetField(tif, TIFFTAG_SOFTWARE,        (const char*)soft.ascii());

            for (y = 0; y < height; y++)
            {
                data = imgData + (y * width * 3);
                TIFFWriteScanline(tif, data, y, 0);
            }
    
            TIFFClose(tif);
            break;
        }
        case RawDecodingSettings::PPM:
        {
            FILE* f = fopen(QFile::encodeName(destPath), "wb");
            if (!f) 
            {
                kdDebug( 51000 ) << "Failed to open ppm file for writing"
                                 << endl;
                delete [] imgData;     
                return false;
            }
    
            fprintf(f, "P6\n%d %d\n255\n", width, height);
            fwrite(imgData, 1, width*height*3, f);
            fclose(f);
            break;
        }
        default:
        {
            kdDebug( 51000 ) << "Invalid output file format"
                             << endl;
            delete [] imgData;     
            return false;
        }
    }
    
    delete [] imgData;     
    return true;
}

}  // namespace KIPIRawConverterPlugin
