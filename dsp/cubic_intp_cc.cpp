#include "cubic_intp_cc.h"
#include <thread>

cubic_intp_cc_sptr cubic_intp_cc_make(double in_samrate, double out_samrate)
{
	return gnuradio::get_initial_sptr(new cubic_intp_cc(in_samrate, out_samrate));
};

cubic_intp_cc::cubic_intp_cc(double in_samrate, double out_samrate)
	: gr::block("cubic_intp_cc",
		gr::io_signature::make(1, 1, sizeof(gr_complex)),
		gr::io_signature::make(1, 1, sizeof(gr_complex))),
	d_in_samrate(in_samrate),
	d_out_samrate(out_samrate)
{
	d_n_coef = 4096;
	d_v_coef.resize(4 * d_n_coef, 0.0f);
	init_cubic_coef(d_v_coef, d_n_coef);
	UINT64 one_std = ((UINT64)1) << 32;
	d_samp_point = one_std;
	set_history(2);//设置历史长度为2，需要保留1个样点
	d_samp_step = static_cast<UINT64>(in_samrate * 1.0/ out_samrate * 1.0* one_std);
	d_ratio = out_samrate / in_samrate;
};

void cubic_intp_cc::init_cubic_coef(std::vector<float>& v_coef, int n_coef) {
	for (int i = 0; i < n_coef; ++i) {
		float x = static_cast<float>(i) / n_coef;
		v_coef[4 * i] = -1.0f / 3.0f * x + 0.5f * x * x - 1.0f / 6.0f * x * x * x; // a0
		v_coef[4 * i + 1] = 1.0f - 0.5f * x - 1.0f * x * x + 0.5f * x * x * x;// a1
		v_coef[4 * i + 2] = 1.0f * x + 0.5f * x * x - 0.5f * x * x * x;// a2 
		v_coef[4 * i + 3] = -1.0 / 6.0 * x + 1.0 / 6.0 * x * x * x;// a3
	}
}

void cubic_intp_cc::forecast(int noutput_items,
	gr_vector_int& ninput_items_required)
{
	unsigned ninputs = ninput_items_required.size();

	for (unsigned i = 0; i < ninputs; i++)
		ninput_items_required[i] = noutput_items / d_ratio + history() - 1 + 2;
};

int cubic_intp_cc::general_work(int noutput_items,
	gr_vector_int& ninput_items,
	gr_vector_const_void_star& input_items,
	gr_vector_void_star& output_items)
{
	gr_complex* in = (gr_complex*)input_items[0];
	gr_complex* out = (gr_complex*)output_items[0];
	int n_input_items = ninput_items[0];
	int nitems_read = n_input_items;
	int nitems = floorf((float)noutput_items / relative_rate());
	int processed = noutput_items;

	consume_each(nitems_read);
	return processed;
}