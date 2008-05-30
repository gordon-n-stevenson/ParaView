/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCTHFragmentCommBuffer.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCTHFragmentCommBuffer.h"
#include "vtkCTHFragmentUtils.hxx"

using vtkstd::vector;

//----------------------------------------------------------------------------
vtkCTHFragmentCommBuffer::vtkCTHFragmentCommBuffer()
{
  // buffer
  this->BufferSize=0;
  this->EOD=0;
  this->Buffer=0;
  // header
  this->HeaderSize=0;
  this->Header=0;
}
//----------------------------------------------------------------------------
vtkCTHFragmentCommBuffer::~vtkCTHFragmentCommBuffer()
{
  this->Clear();
}
//----------------------------------------------------------------------------
void vtkCTHFragmentCommBuffer::Clear()
{ 
  // buffer
  this->BufferSize=0;
  this->EOD=0;
  CheckAndReleaseArrayPointer(this->Buffer);
  // header
  this->HeaderSize=0;
  CheckAndReleaseArrayPointer(this->Header);
}
//----------------------------------------------------------------------------
void vtkCTHFragmentCommBuffer::Initialize(
                int procId,
                int nBlocks,
                int nBytes)
{
  // header
  this->HeaderSize=DESCR_BASE+nBlocks;
  this->Header=new vtkIdType[this->HeaderSize];
  memset(this->Header,0,this->HeaderSize*sizeof(vtkIdType));
  this->Header[PROC_ID]=procId;
  // buffer
  CheckAndReleaseArrayPointer(this->Buffer);
  this->Buffer=new char[nBytes];
  this->Header[BUFFER_SIZE]=nBytes;
  this->EOD=0;
}
//----------------------------------------------------------------------------
void vtkCTHFragmentCommBuffer::SizeHeader(int nBlocks)
{
  // header
  this->HeaderSize=DESCR_BASE+nBlocks;
  this->Header=new vtkIdType[this->HeaderSize];
  memset(this->Header,0,this->HeaderSize*sizeof(vtkIdType));
}
//----------------------------------------------------------------------------
void vtkCTHFragmentCommBuffer::SizeBuffer(int nBytes)
{
  // buffer
  CheckAndReleaseArrayPointer(this->Buffer);
  this->Buffer=new char[nBytes];
  this->Header[BUFFER_SIZE]=nBytes;
  this->EOD=0;
}
//----------------------------------------------------------------------------
void vtkCTHFragmentCommBuffer::SizeBuffer()
{
  // buffer
  CheckAndReleaseArrayPointer(this->Buffer);
  this->Buffer=new char[this->Header[BUFFER_SIZE]];
  this->EOD=0;
  this->BufferSize=this->Header[BUFFER_SIZE];
}
//----------------------------------------------------------------------------
// Append data array to the buffer. Returns the byte index where 
// the array was written.
int vtkCTHFragmentCommBuffer::Pack(vtkDoubleArray *da)
{
  return this->Pack(da->GetPointer(0),
                    da->GetNumberOfComponents(),
                    da->GetNumberOfTuples());
}
//----------------------------------------------------------------------------
// Append data to the buffer. Returns the byte index
// where the array was written to.
int vtkCTHFragmentCommBuffer::Pack(
                const double *pData,
                const int nComps,
                const int nTups)
{
  int byteIdx=this->EOD;

  double *pBuffer
    = reinterpret_cast<double *>(this->Buffer+this->EOD);
  // pack
  for (int i=0; i<nTups; ++i)
    {
    for (int q=0; q<nComps; ++q)
      {
      pBuffer[q]=pData[q];
      }
    pBuffer+=nComps;
    pData+=nComps;
    }
  // update next pack location
  this->EOD+=nComps*nTups*sizeof(double);

  return byteIdx;
}
//----------------------------------------------------------------------------
// Append data to the buffer. Returns the byte index
// where the array was written to.
int vtkCTHFragmentCommBuffer::Pack(
                const int *pData,
                const int nComps,
                const int nTups)
{
  int byteIdx=this->EOD;

  int *pBuffer
    = reinterpret_cast<int *>(this->Buffer+this->EOD);
  // pack
  for (int i=0; i<nTups; ++i)
    {
    for (int q=0; q<nComps; ++q)
      {
      pBuffer[q]=pData[q];
      }
    pBuffer+=nComps;
    pData+=nComps;
    }
  // update next pack location
  this->EOD+=nComps*nTups*sizeof(int);

  return byteIdx;
}
//----------------------------------------------------------------------------
// Pull data from the current location in the buffer 
// into a double array. Before the unpack the array
// is (re)sized.
int vtkCTHFragmentCommBuffer::UnPack(vtkDoubleArray *da,
                  const int nComps, const int nTups, const bool copyFlag)
{
  int ret=0;
  double *pData=0;
  if (copyFlag)
    {
    da->SetNumberOfComponents(nComps);
    da->SetNumberOfTuples(nTups);
    pData=da->GetPointer(0);
    // copy into the buffer
    ret=this->UnPack(pData,nComps,nTups,copyFlag);
    }
  else
    {
    da->SetNumberOfComponents(nComps);
    // get a pointer to the buffer
    ret=this->UnPack(pData,nComps,nTups,copyFlag);
    vtkIdType arraySize=nComps*nTups;
    da->SetArray(pData,arraySize,1);
    }
  return ret;
}

//----------------------------------------------------------------------------
// Extract data from the buffer. Copy flag indicates weather to
// copy or set pointer to buffer.
int vtkCTHFragmentCommBuffer::UnPack(
                double *&rData,
                const int nComps,
                const int nTups,
                const bool copyFlag)
{
  // locate
  double *pBuffer
    = reinterpret_cast<double *>(this->Buffer+this->EOD);

  // unpack
  // copy from buffer
  if (copyFlag)
    {
    double *pData=rData;
    for (int i=0; i<nTups; ++i)
      {
      for (int q=0; q<nComps; ++q)
        {
        pData[q]=pBuffer[q];
        }
      pBuffer+=nComps;
      pData+=nComps;
      }
    }
  // point into buffer
  else
    {
    rData=pBuffer;
    }
  // update next read location
  this->EOD+=nComps*nTups*sizeof(double);

  return 1;
}
//----------------------------------------------------------------------------
// Extract data from the buffer. Copy flag indicates weather to
// copy or set poi9nter to buffer.
int vtkCTHFragmentCommBuffer::UnPack(
                int *&rData,
                const int nComps,
                const int nTups,
                const bool copyFlag)
{
  // locate
  int *pBuffer
    = reinterpret_cast<int *>(this->Buffer+this->EOD);

  // unpack
  // copy from buffer
  if (copyFlag)
    {
    int *pData=rData;
    for (int i=0; i<nTups; ++i)
      {
      for (int q=0; q<nComps; ++q)
        {
        pData[q]=pBuffer[q];
        }
      pBuffer+=nComps;
      pData+=nComps;
      }
    }
  // point into buffer
  else
    {
    rData=pBuffer;
    }
  // update next read location
  this->EOD+=nComps*nTups*sizeof(int);

  return 1;
}
//----------------------------------------------------------------------------
// Set the header size for a vector of buffers.
void vtkCTHFragmentCommBuffer::SizeHeader(
                vector<vtkCTHFragmentCommBuffer> &buffers,
                int nBlocks)
{
  int nBuffers=buffers.size();
  for (int bufferId=0; bufferId<nBuffers; ++bufferId)
    {
    buffers[bufferId].SizeHeader(nBlocks);
    }
}
//----------------------------------------------------------------------------
ostream &operator<<(ostream &sout,const vtkCTHFragmentCommBuffer &fcb)
{
  int hs=fcb.GetHeaderSize();
  sout << "Header size:" << hs << endl;
  int bs=fcb.GetBufferSize();
  sout << "Buffer size:" << bs << endl;
  sout << "EOD:" << fcb.GetEOD() << endl;
  sout << "Header:{";
  const vtkIdType *header=fcb.GetHeader();
  for (int i=0; i<hs; ++i)
    {
    sout << header[i] << ",";
    }
  sout << (char)0x08 << "}" << endl;
  sout << "Buffer:{";
  const int *buffer=reinterpret_cast<const int *>(fcb.GetBuffer());
  bs/=sizeof(int);
  for (int i=0; i<bs; ++i)
    {
    sout << buffer[i] << ",";
    }
  sout << (char)0x08 << "}" << endl;

  return sout;
}

