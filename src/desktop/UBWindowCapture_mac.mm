/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBWindowCapture.h"
#include "UBDesktopAnnotationController.h"

#import <Foundation/NSAutoreleasePool.h>
#import <Carbon/Carbon.h>


UBWindowCapture::UBWindowCapture(UBDesktopAnnotationController *parent) 
	: QObject(parent)
	, mParent(parent)
{
	// NOOP
}


UBWindowCapture::~UBWindowCapture() 
{
	// NOOP
}


const QPixmap UBWindowCapture::getCapturedWindow() 
{
	return mWindowPixmap;
}


int UBWindowCapture::execute()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *path = [NSString pathWithComponents: [NSArray arrayWithObjects: NSTemporaryDirectory(), 
			[[NSString stringWithFormat:@"%d",[NSDate timeIntervalSinceReferenceDate]] stringByAppendingPathExtension:@"uninote"], 
			nil]];
	
    NSTask *task = [[NSTask alloc] init];
	NSArray *arguments = [NSArray arrayWithObjects: @"-i", @"-W", @"-m", @"-tpng", path, nil];
    [task setLaunchPath: @"/usr/sbin/screencapture"];
    [task setArguments: arguments];
	
    [task launch];
    [task waitUntilExit];
    [task release];
	
    QString resultPath = QString::fromUtf8([path UTF8String], strlen([path UTF8String]));
	
    mWindowPixmap.load(resultPath);
    
    QFile::remove(resultPath);
    
    [pool drain];
    
    return QDialog::Accepted;
}

