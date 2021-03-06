// @(#) $Id: AliGPUTPCNeighboursCleaner.cxx 27042 2008-07-02 12:06:02Z richterm $
// **************************************************************************
// This file is property of and copyright by the ALICE HLT Project          *
// ALICE Experiment at CERN, All rights reserved.                           *
//                                                                          *
// Primary Authors: Sergey Gorbunov <sergey.gorbunov@kip.uni-heidelberg.de> *
//                  Ivan Kisel <kisel@kip.uni-heidelberg.de>                *
//                  for The ALICE HLT Project.                              *
//                                                                          *
// Permission to use, copy, modify and distribute this software and its     *
// documentation strictly for non-commercial purposes is hereby granted     *
// without fee, provided that the above copyright notice appears in all     *
// copies and that both the copyright notice and this permission notice     *
// appear in the supporting documentation. The authors make no claims       *
// about the suitability of this software for any purpose. It is            *
// provided "as is" without express or implied warranty.                    *
//                                                                          *
//***************************************************************************

#include "AliGPUTPCNeighboursCleaner.h"
#include "AliGPUTPCTracker.h"
#include "AliTPCCommonMath.h"

GPUd() void AliGPUTPCNeighboursCleaner::Thread(int /*nBlocks*/, int nThreads, int iBlock, int iThread, int iSync,
                                                 GPUsharedref() MEM_LOCAL(AliGPUTPCSharedMemory) & s, GPUconstant() MEM_CONSTANT(AliGPUTPCTracker) & tracker)
{
	// *
	// * kill link to the neighbour if the neighbour is not pointed to the cluster
	// *

	if (iSync == 0)
	{
		if (iThread == 0)
		{
			s.fIRow = iBlock + 2;
			if (s.fIRow <= GPUCA_ROW_COUNT - 3)
			{
				s.fIRowUp = s.fIRow + 2;
				s.fIRowDn = s.fIRow - 2;
				s.fNHits = tracker.Row(s.fIRow).NHits();
			}
		}
	}
	else if (iSync == 1)
	{
		if (s.fIRow <= GPUCA_ROW_COUNT - 3)
		{
#ifdef GPUCA_GPUCODE
			int Up = s.fIRowUp;
			int Dn = s.fIRowDn;
			GPUglobalref() const MEM_GLOBAL(AliGPUTPCRow) &row = tracker.Row(s.fIRow);
			GPUglobalref() const MEM_GLOBAL(AliGPUTPCRow) &rowUp = tracker.Row(Up);
			GPUglobalref() const MEM_GLOBAL(AliGPUTPCRow) &rowDn = tracker.Row(Dn);
#else
			const AliGPUTPCRow &row = tracker.Row(s.fIRow);
			const AliGPUTPCRow &rowUp = tracker.Row(s.fIRowUp);
			const AliGPUTPCRow &rowDn = tracker.Row(s.fIRowDn);
#endif

			// - look at up link, if it's valid but the down link in the row above doesn't link to us remove
			//   the link
			// - look at down link, if it's valid but the up link in the row below doesn't link to us remove
			//   the link
			for (int ih = iThread; ih < s.fNHits; ih += nThreads)
			{
				calink up = tracker.HitLinkUpData(row, ih);
				if (up != CALINK_INVAL)
				{
					calink upDn = tracker.HitLinkDownData(rowUp, up);
					if ((upDn != (calink) ih)) tracker.SetHitLinkUpData(row, ih, CALINK_INVAL);
				}
				calink dn = tracker.HitLinkDownData(row, ih);
				if (dn != CALINK_INVAL)
				{
					calink dnUp = tracker.HitLinkUpData(rowDn, dn);
					if (dnUp != (calink) ih) tracker.SetHitLinkDownData(row, ih, CALINK_INVAL);
				}
			}
		}
	}
}
