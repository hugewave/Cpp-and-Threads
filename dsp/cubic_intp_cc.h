#ifndef CUBIC_INTP_CC_H
#define CUBIC_INTP_CC_H
#include <gnuradio/block.h>
#include <memory>

class cubic_intp_cc; 
typedef std::shared_ptr<cubic_intp_cc> cubic_intp_cc_sptr;
cubic_intp_cc_sptr cubic_intp_cc_make(double in_samrate, double out_samrate);
class cubic_intp_cc : public gr::block
{
public:
	friend cubic_intp_cc_sptr cubic_intp_cc_make(double in_samrate, double out_samrate);
	void forecast(int noutput_items, gr_vector_int& ninput_items_required) override;
	int general_work(int noutput_items,
		gr_vector_int& ninput_items,
		gr_vector_const_void_star& input_items,
		gr_vector_void_star& output_items) override;
protected:
	cubic_intp_cc(double in_samrate, double out_samrate);

private:
	void init_cubic_coef(std::vector<float>& v_coef, int n_coef);
private:
	int d_n_coef; // 在0和1的区间内的采样点数，默认取4096
	double d_in_samrate;
	double d_out_samrate;
	std::vector<float> d_v_coef;
	UINT64 d_samp_step;
	double d_ratio; // 采样率比值,输出比输入
	UINT64 d_samp_point;
};

#endif