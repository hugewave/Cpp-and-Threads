#include <gnuradio/gr_complex.h>
#include <gnuradio/hier_block2.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/top_block.h>
#include <Windows.h>
#include <thread>
#include <string>
#include <ipp.h>
#include "cubic_intp_cc.h"
using namespace std;
int main()
{
	static std::string topblkname = "cubic_inp";
	gr::top_block_sptr tp = gr::make_top_block(topblkname);
	gr::analog::sig_source_c::sptr blk_sine = gr::analog::sig_source_c::make(50000000, gr::analog::GR_SIN_WAVE,
		100000, 1.0);
	double insamrate = 50000000;
	double otsamrate = 60000000;
	cubic_intp_cc_sptr blk_inp = cubic_intp_cc_make(insamrate, otsamrate);
	std::string otfilename = "d:/test.dat";
	gr::blocks::file_sink::sptr blk_filesnk = gr::blocks::file_sink::make(sizeof(gr_complex), otfilename.c_str());

	tp->connect(blk_sine, 0, blk_inp, 0);
	tp->connect(blk_inp, 0, blk_filesnk, 0);
	tp->run();
	return 1;
}
