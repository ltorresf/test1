//
// Copyright 2014 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef INCLUDED_LIBUHD_TX_BLOCK_CTRL_BASE_HPP
#define INCLUDED_LIBUHD_TX_BLOCK_CTRL_BASE_HPP

#include <uhd/usrp/rfnoc/block_ctrl_base.hpp>
#include <uhd/usrp/rfnoc/sink_node_ctrl.hpp>

namespace uhd {
    namespace rfnoc {

/*! \brief Extends block_ctrl_base with input capabilities.
 *
 * A sink block is an RFNoC block that can receive data at an input.
 * We can use this block to transmit data (In RFNoC nomenclature, a
 * transmit operation means streaming data to the device from the host).
 *
 * Every input is defined by a stream signature (stream_sig_t).
 */
class UHD_API sink_block_ctrl_base;
class sink_block_ctrl_base : virtual public block_ctrl_base, public sink_node_ctrl
{
public:
    typedef boost::shared_ptr<sink_block_ctrl_base> sptr;

    /***********************************************************************
     * Streaming operations
     **********************************************************************/
    /*! Set stream args and SID before opening a TX streamer to this block.
     *
     * This is called by the streamer generators. Note this is *not* virtual.
     * To change which settings are set during rx configuration, override _init_tx().
     */
    void setup_tx_streamer(uhd::stream_args_t &args);

    /***********************************************************************
     * Stream signatures
     **********************************************************************/
    /*! Return the input stream signature for a given block port.
     *
     * If \p block_port is not a valid input port, throws
     * a uhd::runtime_error.
     *
     * Calling set_input_signature() can change the return value
     * of this function.
     */
    stream_sig_t get_input_signature(size_t block_port=0) const;

    /*! Tell this block about the stream signature incoming on a given block port.
     *
     * If \p block_port is not a valid output port, throws
     * a uhd::runtime_error.
     *
     * If the input signature is incompatible with this block's signature,
     * it does not throw, but returns false.
     *
     * This function may also affect the output stream signature.
     */
    virtual bool set_input_signature(const stream_sig_t &stream_sig, size_t port=0);

    /***********************************************************************
     * FPGA Configuration
     **********************************************************************/
    /*! Return the size of input buffer on a given block port.
     *
     * This is necessary for setting up flow control, among other things.
     * Note: This does not query the block's settings register. The FIFO size
     * is queried once during construction and cached.
     *
     * If the block port is not defined, it will return 0, and not throw.
     *
     * \param block_port The block port (0 through 15).
     *
     * Returns the size of the buffer in bytes.
     */
    size_t get_fifo_size(size_t block_port=0) const;

    /*! Configure flow control for incoming streams.
     *
     * If flow control is enabled for incoming streams, this block will periodically
     * send out ACKs, telling the upstream block which packets have been consumed,
     * so the upstream block can increase his flow control credit.
     *
     * In the default implementation, this just sets registers
     * SR_FLOW_CTRL_CYCS_PER_ACK and SR_FLOW_CTRL_PKTS_PER_ACK accordingly.
     *
     * Override this function if your block has port-specific flow control settings.
     *
     * \param cycles Send an ACK after this many clock cycles.
     *               Setting this to zero disables this type of flow control acknowledgement.
     * \param packets Send an ACK after this many packets have been consumed.
     *               Setting this to zero disables this type of flow control acknowledgement.
     * \param block_port Set up flow control for a stream coming in on this particular block port.
     */
    virtual void configure_flow_control_in(
            size_t cycles,
            size_t packets,
            size_t block_port=0
     );


protected:
    /***********************************************************************
     * Hooks
     **********************************************************************/
    /*! Before any kind of streaming operation, this will be automatically
     * called to configure the block. Override this to set any tx specific
     * registers etc.
     *
     * Note: \p args may be changed in this function. In a chained operation,
     * the modified value of \p args will be propagated upstream.
     */
    virtual void _init_tx(uhd::stream_args_t &) { /* nop */ };


    /*! Like sink_node_ctrl::_request_input_port(), but also checks
     * the port has an input signature.
     */
    virtual size_t _request_input_port(
            const size_t suggested_port,
            const uhd::device_addr_t &args
    ) const;

}; /* class sink_block_ctrl_base */

}} /* namespace uhd::rfnoc */

#endif /* INCLUDED_LIBUHD_TX_BLOCK_CTRL_BASE_HPP */
// vim: sw=4 et: