/*
 * Copyright (c) 2017, Miroslav Stoyanov
 *
 * This file is part of
 * Toolkit for Adaptive Stochastic Modeling And Non-Intrusive ApproximatioN: TASMANIAN
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * UT-BATTELLE, LLC AND THE UNITED STATES GOVERNMENT MAKE NO REPRESENTATIONS AND DISCLAIM ALL WARRANTIES, BOTH EXPRESSED AND IMPLIED.
 * THERE ARE NO EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE OF THE SOFTWARE WILL NOT INFRINGE ANY PATENT,
 * COPYRIGHT, TRADEMARK, OR OTHER PROPRIETARY RIGHTS, OR THAT THE SOFTWARE WILL ACCOMPLISH THE INTENDED RESULTS OR THAT THE SOFTWARE OR ITS USE WILL NOT RESULT IN INJURY OR DAMAGE.
 * THE USER ASSUMES RESPONSIBILITY FOR ALL LIABILITIES, PENALTIES, FINES, CLAIMS, CAUSES OF ACTION, AND COSTS AND EXPENSES, CAUSED BY, RESULTING FROM OR ARISING OUT OF,
 * IN WHOLE OR IN PART THE USE, STORAGE OR DISPOSAL OF THE SOFTWARE.
 */

#ifndef __TASMANIAN_ADDONS_MPIDREAMSCATTER_HPP
#define __TASMANIAN_ADDONS_MPIDREAMSCATTER_HPP

/*!
 * \internal
 * \file tsgMPIScatterDREAM.hpp
 * \brief DREAM objects send/receive through MPI.
 * \author Miroslav Stoyanov
 * \ingroup TasmanianAddonsCommon
 *
 * Templates that communicate DREAM objects through an MPI commands.
 * \endinternal
 */

#include "tsgMPIScatterGrid.hpp"

/*!
 * \ingroup TasmanianAddons
 * \addtogroup TasmanianAddonsMPIDREAMSend MPI Send/Receive/Scatter for DREAM
 *
 * Methods to send/receive DREAM likelihood objects.
 * The syntax mimics the raw MPI_Send and MPI_Recv calls,
 * and the templates require Tasmanian_ENABLE_MPI=ON.
 */

#ifdef Tasmanian_ENABLE_MPI

namespace TasDREAM{

/*!
 * \ingroup TasmanianAddonsMPIDREAMSend
 * \brief Send a likelihood to another process in the MPI comm.
 *
 * Works with both isotropic and anisotropic Gaussian likelihood
 * object implemented in Tasmanian.
 * The usage is very similar to MPI_Send() and TasGrid::MPIGridSend().
 *
 * \tparam Likelihood is a Tasmanian likelihood class, currently
 *                    LikeliTasDREAM::LikelihoodGaussIsotropic and
 *                    LikeliTasDREAM::LikelihoodGaussAnisotropic.
 *
 * \param likely      is the likelihood to send.
 * \param destination is the rank of the recipient MPI process.
 * \param tag         is the tag to use for the MPI message.
 * \param comm        is the MPI comm where the source and destination reside.
 *
 * \return the error code of the MPI_Send() command.
 *
 * \b Note: this call must be mirrored by TasDREAM::MPILikelihoodRecv() on the
 *          destination process.
 */
template<class Likelihood>
int MPILikelihoodSend(Likelihood const &likely, int destination, int tag, MPI_Comm comm){
    std::stringstream ss;
    likely.write(ss);
    while(ss.str().size() % 16 != 0) ss << " ";
    return MPI_Send(ss.str().c_str(), (int) (ss.str().size() / 16), MPI_LONG_DOUBLE, destination, tag, comm);
}

/*!
 * \ingroup TasmanianAddonsMPIDREAMSend
 * \brief Receive a likelihood from another process in the MPI comm.
 *
 * Works with both isotropic and anisotropic Gaussian likelihood
 * object implemented in Tasmanian.
 * The usage is very similar to MPI_Recv() and TasGrid::MPIGridRecv().
 *
 * \tparam Likelihood is a Tasmanian likelihood class, currently
 *                    LikeliTasDREAM::LikelihoodGaussIsotropic and
 *                    LikeliTasDREAM::LikelihoodGaussAnisotropic.
 *
 * \param likely   is the output likelihood, it will be overwritten with the one send.
 * \param source   is the rank of the process in the MPI comm that issued the send command.
 * \param tag      is the tag used in the MPI send command.
 * \param comm     is the MPI comm where the source and destination reside.
 * \param status   is the status of the MPI_Recv() command.
 *
 * \return the error code of the MPI_Recv() command.
 *
 * \b Note: see TasDREAM::MPILikelihoodSend().
 */
template<class Likelihood>
int MPILikelihoodRecv(Likelihood &likely, int source, int tag, MPI_Comm comm, MPI_Status *status = MPI_STATUS_IGNORE){
    MPI_Status internal_status;
    if (status == MPI_STATUS_IGNORE) status = &internal_status;

    int short_data_size;
    MPI_Probe(source, tag, comm, status);
    MPI_Get_count(status, MPI_LONG_DOUBLE, &short_data_size);

    size_t data_size = TasGrid::Utils::size_mult(short_data_size, 16);

    std::vector<char> buff(data_size);
    auto result = MPI_Recv(buff.data(), (int) (data_size / 16), MPI_LONG_DOUBLE, source, tag, comm, status);

    TasGrid::VectorToStreamBuffer data_buffer(buff); // do not modify buff after this point
    std::istream is(&data_buffer);
    likely.read(is);
    return result;
}

}

#endif // Tasmanian_ENABLE_MPI

#endif
