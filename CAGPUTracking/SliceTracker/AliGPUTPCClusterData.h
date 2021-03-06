// **************************************************************************
// * This file is property of and copyright by the ALICE HLT Project        *
// * All rights reserved.                                                   *
// *                                                                        *
// * Primary Authors:                                                       *
// *     Copyright 2009       Matthias Kretz <kretz@kde.org>                *
// *                                                                        *
// * Permission to use, copy, modify and distribute this software and its   *
// * documentation strictly for non-commercial purposes is hereby granted   *
// * without fee, provided that the above copyright notice appears in all   *
// * copies and that both the copyright notice and this permission notice   *
// * appear in the supporting documentation. The authors make no claims     *
// * about the suitability of this software for any purpose. It is          *
// * provided "as is" without express or implied warranty.                  *
// **************************************************************************

#ifndef ALIHLTTPCCACLUSTERDATA_H
#define ALIHLTTPCCACLUSTERDATA_H

#include "AliGPUTPCDef.h"

#if !defined(__OPENCL__)
#include <iostream>
#include <vector>

/**
 * Cluster data which keeps history about changes
 *
 * The algorithm doesn't work on this data. Instead the AliGPUTPCSliceData is created from this.
 */
class AliGPUTPCClusterData
{
  public:

	AliGPUTPCClusterData(): fSliceIndex( 0 ), fData( NULL ), fNumberOfClusters(0), fAllocated(0) {}
	~AliGPUTPCClusterData();

    struct Data {
      int fId;
      short fRow;
      short fFlags;
      float fX;
      float fY;
      float fZ;
      float fAmp;
#ifdef GPUCA_FULL_CLUSTERDATA
      float fPad;
      float fTime;
      float fAmpMax;
      float fSigmaPad2;
      float fSigmaTime2;
#endif
    };

    /**
     * prepare for the reading of event
     */
    void StartReading( int sliceIndex, int guessForNumberOfClusters );

    Data* Clusters() { return(fData); }
    void SetNumberOfClusters(int number) {fNumberOfClusters = number;}

    /**
     * Read/Write Events from/to file
     */
    void ReadEvent(std::istream &in, bool addData = false);
    void WriteEvent(std::ostream &out) const;
    template <class T> void ReadEventVector(T* &data, std::istream &in, int MinSize = 0, bool addData = false);
    template <class T> void WriteEventVector(const T* const &data, std::ostream &out) const;

    /**
     * The slice index this data belongs to
     */
    int SliceIndex() const { return fSliceIndex; }

    /**
     * Return the number of clusters in this slice.
     */
    int NumberOfClusters() const { return (int) fNumberOfClusters; }

    /**
     * Return the x coordinate of the given cluster.
     */
    float X( int index ) const { return fData[index].fX; }

    /**
     * Return the y coordinate of the given cluster.
     */
    float Y( int index ) const { return fData[index].fY; }

    /**
     * Return the z coordinate of the given cluster.
     */
    float Z( int index ) const { return fData[index].fZ; }

    /**
     * Return the amplitude of the given cluster.
     */
    float Amp( int index ) const { return fData[index].fAmp; }

    /**
     * Return the global ID of the given cluster.
     */
    int Id( int index ) const { return fData[index].fId; }

    /**
     * Return the row number/index of the given cluster.
     */
    short Flags( int index ) const { return fData[index].fFlags; }
    int RowNumber( int index ) const { return fData[index].fRow; }

    Data *GetClusterData( int index ) { return &( fData[index] ); }

    void Allocate( int number);
    
    void SetClusterData(int sl, int n, const Data* d);

  private:
    AliGPUTPCClusterData(AliGPUTPCClusterData&): fSliceIndex( 0 ), fData( NULL ), fNumberOfClusters(0), fAllocated(0) {}
    AliGPUTPCClusterData& operator=( const AliGPUTPCClusterData& );

    static bool CompareClusters( const Data &a, const Data &b ) { return ( a.fRow == b.fRow ? (a.fY < b.fY) : (a.fRow < b.fRow) ); }

    int fSliceIndex;  // the slice index this data belongs to
    Data* fData; // list of data of clusters
    int fNumberOfClusters;	//Current number of clusters stored in fData
    int fAllocated; //Number of clusters that can be stored in fData
};

typedef AliGPUTPCClusterData ClusterData;

#endif

#endif // CLUSTERDATA_H
