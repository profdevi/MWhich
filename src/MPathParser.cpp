/*    
    MWhich.exe : Searches for files using the PATH environment variables 
                 or specified environment variable 
    Copyright (C) 2017  Comine.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

//v1.10 copyright Comine.com 20160718M0708
#include <windows.h>
#include "MStdLib.h"
#include "MBuffer.h"
#include "MPathParser.h"


//******************************************************
//**  Module Elements
//******************************************************
static const int GMaxBufferSize=_MAX_PATH*6;

////////////////////////////////////////////////////////
static bool GIsDriveLetter(char ch)
	{
	if(ch>='A' && ch<='Z') {  return true; }
	if(ch>='a' && ch<='z') { return true; }
	return false;
	}


////////////////////////////////////////////////////////
static int GGetDotCount(const char *filename)
	{
	if(filename==NULL)
		{
		return 0;
		}

	int count=0;
	for(int i=0;filename[i]!=0;++i)
		{
		if(filename[i]=='.') { count=count+1; }
		}

	return count;
	}


///////////////////////////////////////////////////////
static int GGetDotPosition(const char *filename)
	{
	if(filename==NULL)
		{
		return 0;
		}

	for(int i=MStdStrLen(filename)-1;i>=0;--i)
		{
		if(filename[i]=='.') { return i; }
		}

	return 0;
	}


////////////////////////////////////////////////////////////
static bool GMakePath(char *path,int buffersize,const char *drive
		,const char *dir,const char *fname,const char *fext)
	{
	#if _MSC_VER>=1300
	if(_makepath_s(path,buffersize,drive,dir,fname,fext)!=0)
		{
		return false;
		}

	return true;
	#else	// _MSC_VER
	_makepath(path,drive,dir,fname,fext);
	return true;
	#endif // _MSC_VER
	}


//******************************************************
//**  MPathParser class
//******************************************************
void MPathParser::ClearObject(void)
	{
	mDrive=NULL;
	mDir=NULL;
	mFileName=NULL;
	mFileExt=NULL;
	mPath=NULL;
	mFullFileName=NULL;
	}


/////////////////////////////////////////////////
void MPathParser::MakePathNames(void)	// Make a pathname
	{
	if(GMakePath(mPath,_MAX_PATH,mDrive,mDir,mFileName,mFileExt)==false)
		{
		return;
		}

	// Set Full File Name
	MStdSPrintf(mFullFileName,_MAX_PATH,"%s%s",mFileName,mFileExt);	
	}

/////////////////////////////////////////////////
MPathParser::MPathParser(bool init)
	{
	ClearObject();
	if(init==true && Create()==false)
		{
		return;
		}
	}


////////////////////////////////////////////////
MPathParser::~MPathParser(void)
	{  Destroy();  }


////////////////////////////////////////////////
bool MPathParser::Create(void)
	{
	Destroy();
	if(mBuffer.Create(GMaxBufferSize)==false)
		{
		Destroy();  return false;
		}

	mBuffer.Clear(); // Init buffer

	// Set up elements to empty string
	mPath=mBuffer.GetBuffer();
	mDrive=mPath+_MAX_PATH;
	mDir=mDrive+_MAX_PATH;
	mFileName=mDir+_MAX_PATH;
	mFileExt=mFileName+_MAX_PATH;
	mFullFileName=mFileExt+_MAX_PATH;

	return true;
	}


////////////////////////////////////////////////
bool MPathParser::Destroy(void)
	{
	mBuffer.Destroy();
	ClearObject();
	return true;
	}


/////////////////////////////////////////////////////
bool MPathParser::SetDrive(const char *drive)
	{
	if(drive==NULL || MStdStrLen(drive)>=_MAX_PATH)
		{
		return false;
		}

	MStdStrCpy(mDrive,drive);
	MakePathNames();
	return true;
	}


/////////////////////////////////////////////////////
bool MPathParser::SetDir(const char *dir)
	{
	if(dir==NULL || MStdStrLen(dir)>=_MAX_PATH)
		{
		return false;
		}	

	MStdStrCpy(mDir,dir);
	MakePathNames();
	return true;
	}


//////////////////////////////////////////////////////
bool MPathParser::SetDriveDir(const char *dir)				// Set the Dir Path with drive
	{
	int dirlength=MStdStrLen(dir);
	if(dir==NULL || dirlength>=_MAX_PATH || dirlength<=2)
		{
		return false;
		}
	
	if(GIsDriveLetter(dir[0])==false || dir[1]!=':')
		{
		return false;		
		}

	// Copy Drive letter
	mDir[0]=dir[0];  mDir[1]=':';  mDir[2]=0;

	// Copy Path
	MStdStrCpy(mDir,dir+2);

	mDrive[0]=dir[0];
	mDrive[1]=':';
	mDrive[2]=0;

	MakePathNames();  return true;
	}


/////////////////////////////////////////////////////
bool MPathParser::SetFileName(const char *filename)
	{
	if(filename==NULL || MStdStrLen(filename)>=_MAX_PATH)
		{
		return false;
		}

	MStdStrCpy(mFileName,filename);
	MakePathNames();
	return true;
	}


/////////////////////////////////////////////////////
bool MPathParser::SetFileExt(const char *extension)
	{
	if(extension==NULL || MStdStrLen(extension)>=_MAX_PATH)
		{
		return false;
		}

	MStdStrCpy(mFileExt,extension);
	MakePathNames();
	return true;
	}


/////////////////////////////////////////////////////
bool MPathParser::SetPath(const char *path)
	{
	MBuffer tmppath;
	if(tmppath.Create(GMaxBufferSize)==false)
		{
		return false;
		}

	// Set path
	tmppath.SetString(path);

	_fullpath(mPath,tmppath,_MAX_PATH);

	#if _MSC_VER>=1300
	if(_splitpath_s(mPath,mDrive,_MAX_PATH,mDir,_MAX_PATH
			,mFileName,_MAX_PATH,mFileExt,_MAX_PATH)!=0)
		{
		return false;
		}
	#else // _MSC_VER
	_splitpath(mPath,mDrive,mDir,mFileName,mFileExt);
	#endif // _MSC_VER

	MakePathNames();

	return true;
	}


/////////////////////////////////////////////////////
bool MPathParser::SetFullFileName(const char *filename)	// Set Filename as "One.txt";
	{
	if(filename==NULL)
		{
		return false;
		}

	int dotcount=GGetDotCount(filename);
	if(dotcount!=1)
		{
		//=We have a simple file name
		return SetFileName(filename);
		}

	int length=MStdStrLen(filename);
	int dotposition=GGetDotPosition(filename);

	if(length-dotposition-1>3)
		{
		//=File Extension is not three chars
		return SetFileName(filename); // Set to normal filename
		}

	// Check if file length is good
	if(length>=_MAX_PATH)
		{
		return false;
		}

	//Copy file name to temp buffer
	MBuffer buffer;
	if(buffer.Create(_MAX_PATH+1)==false)
		{
		return false;
		}

	MStdMemCpy(buffer.GetBuffer(),filename,dotposition);
	buffer[dotposition]=0;

	MStdStrCpy(mFileName,buffer.GetBuffer());

	// Copy File Extension
	MStdStrCpy(mFileExt,filename+dotposition);

	GMakePath(mPath,_MAX_PATH,mDrive,mDir,mFileName,mFileExt);

	// Set Full File Name
	MStdSPrintf(mFullFileName,_MAX_PATH,"%s%s",mFileName,mFileExt);

	return true;
	}


/////////////////////////////////////////////////////
const char *MPathParser::GetDrive(void)
	{
	return mDrive;
	}


///////////////////////////////////////////////////////
const char *MPathParser::GetDir(void)
	{
	return mDir;
	}


///////////////////////////////////////////////////////
const char *MPathParser::GetFileName(void)
	{
	return mFileName;
	}


///////////////////////////////////////////////////////
const char *MPathParser::GetFileExt(void)
	{
	return mFileExt;
	}


//////////////////////////////////////////////////////
const char *MPathParser::GetPath(void)
	{
	return mPath;
	}


//////////////////////////////////////////////////////
const char *MPathParser::GetFullFileName(void)		// Get Full File name(ie "one.txt")
	{
	return mFullFileName;
	}


//////////////////////////////////////////////////////
bool MPathParser::Clear(void)
	{
	*mDrive=0;  *mDir=0;  *mFileName=0; *mFileExt=0;  *mPath=0;
	return true;
	}


//////////////////////////////////////////////////////
bool MPathParser::SetFileNameInExeHome(const char *filename)
	{
	MBuffer buffer;
	if(buffer.Create(GMaxBufferSize)==false)
		{
		return false;
		}

	if( GetModuleFileNameA(NULL,buffer.GetBuffer(),buffer.GetSize() )==0)
		{
		return false;
		}
	

	MPathParser temppath;
	if(temppath.Create()==false)
		{
		return false;
		}

	MBuffer tmpbuffer(GMaxBufferSize);
	MStdSPrintf(tmpbuffer,tmpbuffer.GetSize(),"%s",buffer.GetBuffer());

	if(temppath.SetPath(tmpbuffer)==false)
		{
		return false;
		}

	MStdSPrintf(tmpbuffer,tmpbuffer.GetSize()
				,"%s%s%s",temppath.GetDrive(),temppath.GetDir()
				,filename);
	if(SetPath(tmpbuffer)==false)
		{
		return false;
		}

	return true;
	}


