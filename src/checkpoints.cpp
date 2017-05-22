// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (   0, uint256("0x563ac70cc2642286ad8463559011621fc4debe7ab2525900f74d079fc73cb5f2"))
 /*       (   1, uint256("0xedcce7a202f07ea4ea2ca1883b7d70c6f44fa53f5f88ba62abe8f94284a1d7b3"))
        ( 801, uint256("0xb3a950b3d94c2d1298cacc9089c8e3ba90fb425306cfbf04cc39282ac6a794d2")) // chain revert to fix zerocoin
        (1306, uint256("0x0413b8006c6eaf596ed3acb9ef50c9e5df7fd88bb6c5202a39c2f7dc70ca886c"))
        (2328, uint256("0x2cc7f3e50b375a6a0fa436bc70cbfdd734eb8379aaf9e1afcbb1c9bcac06d853"))
        (2688, uint256("0x702449330dee0d3d440442aba693cf8f552c0d78cf2d50adc99bc49255968d04"))
        (4460, uint256("0xd73d6126bf876454531967c5da2d85ddc9e4069844f6873bb5e127ad55fec45c"))
        (8065, uint256("0xf199acbb763950c21aaafeaaa9e36b9564febcf8b2d876603ac3a9a66d0932ff"))
        (8153, uint256("0x9b677ec2316798d44ff8bb364a6cd60ef2b44015848be3605bb71d2a5e8b3066"))
        (8154, uint256("0x93cf3697a6b001dd2682af40c2643689b0eab3ac27c86c791323f6a287b4b169"))
        (9127, uint256("0xbb8a0ea3ecdf500a3f7da9014503db7b4accb6b2538bc922af629ecaf5bf301b"))
        (11144, uint256("0xe0fb2ef28623b8e303fe051f4aac4b9378f14c17043ca62413f7894552c5a90f"))
        (12222, uint256("0x513ddfe0e55e53442cd2659612f5ff0516c7022bb126f932c5aec64d6ac08af5"))
		(15001, uint256("0xc84d91a83ec6fa779e607fa7403e8708318f321c8364c6686205f7e70900cb98"))	
		(19810, uint256("0xea3a0d7e61f8e67cc51ed7e9b4044974bb88c44a3650917e92713ad94bc11db5"))
		(30001, uint256("0x223dca0c2a6fd028dc4df4b5b4309985502ee839bc7dcd968494368007866540"))
*/
	;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1489231307, // * UNIX timestamp of last checkpoint block
        23062,    // * total number of transactions between genesis and last checkpoint
                  //   (the tx=... number in the SetBestChain debug.log lines)
        576.0     // * estimated number of transactions per day after checkpoint
    };


    const CCheckpointData &Checkpoints() {

            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
