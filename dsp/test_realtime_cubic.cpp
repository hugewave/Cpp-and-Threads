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
#include <ipp.h>

void init_cubic_coef(std::vector<float>& v_coef, int n_coef) {
  for (int i = 0; i < n_coef; ++i) {
    float x = static_cast<float>(i) / n_coef;
    v_coef[4 * i] =  - 1.0f/3.0f * x + 0.5f*x * x - 1.0f/6.0f * x * x * x; // a0
    v_coef[4 * i + 1] = 1.0f -0.5f*x -1.0f*x*x + 0.5f*x*x*x;// a1
    v_coef[4 * i + 2] = 1.0f*x + 0.5f*x*x -0.5f*x*x*x;// a2 
    v_coef[4 * i + 3] = -1.0/6.0*x + 1.0/6.0*x * x * x;// a3
  }
}

void init_cubic_coef_16s(std::vector<Ipp16s>& v_coef, int n_coef) {
	std::vector<float> v_coef_32f(n_coef * 4);
	init_cubic_coef(v_coef_32f, n_coef);
	for (int i = 0; i < n_coef * 4; ++i) {
		v_coef[i] = v_coef_32f[i] * (1 << 16);
	}
}

void init_cubic_coef_32s(std::vector<Ipp16s>& v_coef, int n_coef) {
	std::vector<float> v_coef_32f(n_coef * 4);
	init_cubic_coef(v_coef_32f, n_coef);
	for (int i = 0; i < n_coef * 4; ++i) {
		v_coef[i] = v_coef_32f[i] * (1 << 16);
	}
}

void fun_cubic(Ipp32f* indatabuf, Ipp32f* outdatabuf, Ipp32f* coefbuf, UINT64 ini_pos, UINT64 samp_step, int n_samples)
{
	int idx_out = 0;
	UINT64 pos = ini_pos;
	int samp_idx = (pos >> 32) - 1;
	int coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
	for (int i = 0; i < n_samples; ++i)
	{
		ippsDotProd_32f(coefbuf + coef_idx, indatabuf + samp_idx, 4, outdatabuf + idx_out);
		pos += samp_step;
		samp_idx = (pos >> 32) - 1;
		coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
		idx_out++;
	}
};

void fun_cubic_complex(Ipp32fc* indatabuf, Ipp32fc* outdatabuf, Ipp32f* coefbuf, UINT64 ini_pos, UINT64 samp_step, int n_samples)
{
	int idx_out = 0;
	UINT64 pos = ini_pos;
	int samp_idx = (pos >> 32) - 1;
	int coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
	for (int i = 0; i < n_samples; ++i)
	{
		ippsDotProd_32f32fc(coefbuf + coef_idx, indatabuf + samp_idx, 4, outdatabuf + idx_out);
		pos += samp_step;
		samp_idx = (pos >> 32) - 1;
		coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
		idx_out++;
	}
};

void fun_cubic_16sc(Ipp16sc* indatabuf, Ipp16sc* outdatabuf, Ipp16s* coefbuf, UINT64 ini_pos, UINT64 samp_step, int n_samples)
{
	int idx_out = 0;
	UINT64 pos = ini_pos;
	int samp_idx = (pos >> 32) - 1;
	int coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
	Ipp64sc* tmpbuf_64s = ippsMalloc_64sc(n_samples);
	Ipp32sc* tmpbuf_32s = ippsMalloc_32sc(n_samples);
	for (int i = 0; i < n_samples; ++i)
	{
		ippsDotProd_16s16sc64sc(coefbuf + coef_idx, indatabuf + samp_idx, 4, tmpbuf_64s + idx_out);
		pos += samp_step;
		samp_idx = (pos >> 32) - 1;
		coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
		idx_out++;
	}
	ippsConvert_64s32s_Sfs((Ipp64s*)tmpbuf_64s, (Ipp32s*)tmpbuf_32s, n_samples * 2, ippRndNear, -16);
	ippsConvert_32s16s((Ipp32s*)tmpbuf_32s, (Ipp16s*)outdatabuf, n_samples * 2);
	ippsFree(tmpbuf_64s);
	ippsFree(tmpbuf_32s);
};

void cubic_float_address()
{
	int n_coef = 4096;
	int len_cubic_coef = 4 * n_coef;
	std::vector<float> v_coef(len_cubic_coef, 0.0f);
	init_cubic_coef(v_coef, n_coef);

	int n_samples = 10000000;
	std::vector<float> v_samples(n_samples, 0.0f);
	for (int i = 0; i < n_samples; ++i)
	{
		v_samples[i] = static_cast<float>(i) / n_samples;
	}

	double in_samrate = 50000000;
	double out_samrate = 60000000;
	double ratio = out_samrate / in_samrate;
	double samp_step = in_samrate / out_samrate;
	int n_out_samples = static_cast<int>(n_samples * ratio);
	std::vector<float> v_out_samples(n_out_samples, 0.0f);

	double ini_pos = 0;
	double pos = 1.0;
	int samp_idx = pos - 1;
	int coef_idx = (pos - 1 - samp_idx) * 4096;
	int idx_out = 0;
	double tic = GetTickCount();

	while (samp_idx < (n_samples - 2))
	{
		v_out_samples[idx_out] = 0.0f;
		for (int i = 0; i < 4; ++i)
		{
			v_out_samples[idx_out] += v_coef[coef_idx] * v_samples[samp_idx];
			coef_idx++;
			samp_idx++;
		}
		pos += samp_step;
		samp_idx = pos - 1;
		coef_idx = (pos - 1 - samp_idx) * 4096 * 4;
		idx_out++;
	}

	double toc = GetTickCount();
	std::cout << "signal time:" << n_samples * 1000.0 / in_samrate << std::endl;
	std::cout << "Time taken: " << toc - tic << " ms" << std::endl;
	double t_ori = n_samples * 1000.0 / in_samrate;
	double t_task = (toc - tic);
	;
	std::cout << "cubic_float_address time ratio: " << t_task / t_ori << std::endl;
}

void cubic_int_address()
{
	int n_coef = 4096;
	int len_cubic_coef = 4 * n_coef;
	std::vector<float> v_coef(len_cubic_coef, 0.0f);
	init_cubic_coef(v_coef, n_coef);

	int n_samples = 10000000;
	std::vector<float> v_samples(n_samples, 0.0f);
	for (int i = 0; i < n_samples; ++i)
	{
		v_samples[i] = static_cast<float>(i) / n_samples;
	}

	double in_samrate = 50000000;
	double out_samrate = 60000000;
	double ratio = out_samrate / in_samrate;
	int n_out_samples = static_cast<int>(n_samples * ratio);
	std::vector<float> v_out_samples(n_out_samples, 0.0f);
	UINT64 one_std = ((UINT64)1) << 32;
	UINT64 samp_step = static_cast<UINT64>(in_samrate * 1.0 / out_samrate * 1.0 * one_std);

	UINT64 ini_pos = 1;
	UINT64 pos = ini_pos << 32 - 1;
	int samp_idx = (pos >> 32);
	int coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
	int idx_out = 0;
	double tic = GetTickCount();

	while (samp_idx < (n_samples - 2))
	{
		v_out_samples[idx_out] = 0.0f;
		for (int i = 0; i < 4; ++i)
		{
			v_out_samples[idx_out] += v_coef[coef_idx] * v_samples[samp_idx];
			coef_idx++;
			samp_idx++;
		}
		pos += samp_step;
		samp_idx = (pos >> 32) - 1;
		coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
		idx_out++;
	}

	double toc = GetTickCount();
	std::cout << "signal time:" << n_samples * 1000.0 / in_samrate << std::endl;
	std::cout << "Time taken: " << toc - tic << " ms" << std::endl;
	double t_ori = n_samples * 1000.0 / in_samrate;
	double t_task = (toc - tic);
	;
	std::cout << "cubic_int_address:Time ratio: " << t_task / t_ori << std::endl;
};

void cubic_ipp_int_address()
{
	int n_coef = 4096;
	int len_cubic_coef = 4 * n_coef;
	std::vector<float> v_coef(len_cubic_coef, 0.0f);
	init_cubic_coef(v_coef, n_coef);

	int n_samples = 10000000;
	std::vector<float> v_samples(n_samples, 0.0f);
	for (int i = 0; i < n_samples; ++i)
	{
		v_samples[i] = static_cast<float>(i) / n_samples;
	}

	double in_samrate = 50000000;
	double out_samrate = 60000000;
	double ratio = out_samrate / in_samrate;
	int n_out_samples = static_cast<int>(n_samples * ratio);
	std::vector<float> v_out_samples(n_out_samples, 0.0f);
	UINT64 one_std = ((UINT64)1) << 32;
	UINT64 samp_step = static_cast<UINT64>(in_samrate * 1.0 / out_samrate * 1.0 * one_std);

	UINT64 ini_pos = 1;
	UINT64 pos = (ini_pos << 32);
	int samp_idx = (pos >> 32) - 1;
	int coef_idx = ((pos & 0x00000000FFFFFFFF) >> 20) << 2;
	int idx_out = 0;
	double tic = GetTickCount();

	while (samp_idx < (n_samples - 2))
	{
		ippsDotProd_32f(v_coef.data() + coef_idx, v_samples.data() + samp_idx, 4, v_out_samples.data() + idx_out);
		pos += samp_step;
		samp_idx = static_cast<int>(pos >> 32) - 1;
		coef_idx = static_cast<int>((pos & 0xFFFFFFFF) >> 20) << 2;
		idx_out++;
	}

	double toc = GetTickCount();
	std::cout << "signal time:" << n_samples * 1000.0 / in_samrate << std::endl;
	std::cout << "Time taken: " << toc - tic << " ms" << std::endl;
	double t_ori = n_samples * 1000.0 / in_samrate;
	double t_task = (toc - tic);
	;
	std::cout << "cubic_ipp_int_address:Time ratio: " << t_task / t_ori << std::endl;
};

void cubic_multithread_ipp_int_address()
{
	int n_coef = 4096;
	int len_cubic_coef = 4 * n_coef;
	std::vector<float> v_coef(len_cubic_coef, 0.0f);
	init_cubic_coef(v_coef, n_coef);

	int n_samples = 10000000;
	std::vector<float> v_samples(n_samples, 0.0f);
	for (int i = 0; i < n_samples; ++i)
	{
		//v_samples[i] = static_cast<float>(i) / n_samples;
		v_samples[i] = static_cast<float>(i);
	}
	double in_samrate = 50000000;
	double out_samrate = 60000000;
	double ratio = out_samrate / in_samrate;
	int n_out_samples = static_cast<int>(n_samples * ratio) - 10;
	std::vector<float> v_out_samples(n_out_samples, 0.0f);
	UINT64 one_std = ((UINT64)1) << 32;
	UINT64 samp_step = static_cast<UINT64>(in_samrate * 1.0 / out_samrate * 1.0 * one_std);

	double tic = GetTickCount();
	int n_threads = 8;
	int n_samples_per_thread = n_out_samples / n_threads;
	std::vector<std::thread> threads(n_threads);
	for (int i = 0; i < n_threads; ++i)
	{
		threads[i] = std::thread([&, i]() {
			UINT64 pos = one_std + n_samples_per_thread * samp_step * i;
			fun_cubic(v_samples.data(), v_out_samples.data() + n_samples_per_thread * i, v_coef.data(), pos, samp_step, n_samples_per_thread);
			});
	}

	for (auto& thread : threads)
		thread.join();

	double toc = GetTickCount();
	std::cout << "signal time:" << n_samples * 1000.0 / in_samrate << std::endl;
	std::cout << "Time taken: " << toc - tic << " ms" << std::endl;
	double t_ori = n_samples * 1000.0 / in_samrate;
	double t_task = (toc - tic);
	;
	std::cout << "cubic_multithread_ipp_int_address:Time ratio: " << t_task / t_ori << std::endl;
}

void cubic_complex_multithread_ipp_int_address()
{
	int n_coef = 4096;
	int len_cubic_coef = 4 * n_coef;
	std::vector<float> v_coef(len_cubic_coef, 0.0f);
	init_cubic_coef(v_coef, n_coef);

	int n_samples = 10000000;
	std::vector<Ipp32fc> v_samples(n_samples);
	for (int i = 0; i < n_samples; ++i)
	{
		//v_samples[i] = static_cast<float>(i) / n_samples;
		v_samples[i].re = static_cast<float>(i);
		v_samples[i].im = static_cast<float>(i);
	}
	double in_samrate = 50000000;
	double out_samrate = 60000000;
	double ratio = out_samrate / in_samrate;
	int n_out_samples = static_cast<int>(n_samples * ratio) - 10;
	std::vector<Ipp32fc> v_out_samples(n_out_samples);
	UINT64 one_std = ((UINT64)1) << 32;
	UINT64 samp_step = static_cast<UINT64>(in_samrate * 1.0 / out_samrate * 1.0 * one_std);

	double tic = GetTickCount();
	int n_threads = 8;
	int n_samples_per_thread = n_out_samples / n_threads;
	std::vector<std::thread> threads(n_threads);
	for (int i = 0; i < n_threads; ++i)
	{
		threads[i] = std::thread([&, i]() {
			UINT64 pos = one_std + n_samples_per_thread * samp_step * i;
			fun_cubic_complex(v_samples.data(), v_out_samples.data() + n_samples_per_thread * i, v_coef.data(), pos, samp_step, n_samples_per_thread);
			});
	}

	for (auto& thread : threads)
		thread.join();

	double toc = GetTickCount();
	std::cout << "signal time:" << n_samples * 1000.0 / in_samrate << std::endl;
	std::cout << "Time taken: " << toc - tic << " ms" << std::endl;
	double t_ori = n_samples * 1000.0 / in_samrate;
	double t_task = (toc - tic);
	;
	std::cout << "cubic_complex_multithread_ipp_int_address:Time ratio: " << t_task / t_ori << std::endl;
};

void cubic_16sc_multithread_ipp_int_address()
{
	int n_coef = 4096;
	int len_cubic_coef = 4 * n_coef;
	std::vector<Ipp16s> v_coef(len_cubic_coef, 0);
	init_cubic_coef_16s(v_coef, n_coef);

	int n_samples = 10000000;
	std::vector<Ipp16sc> v_samples(n_samples);
	for (int i = 0; i < n_samples; ++i)
	{
		//v_samples[i] = static_cast<float>(i) / n_samples;
		v_samples[i].re = static_cast<short>(i);
		v_samples[i].im = static_cast<short>(i);
	}
	double in_samrate = 50000000;
	double out_samrate = 60000000;
	double ratio = out_samrate / in_samrate;
	int n_out_samples = static_cast<int>(n_samples * ratio) - 10;
	std::vector<Ipp16sc> v_out_samples(n_out_samples);
	UINT64 one_std = ((UINT64)1) << 32;
	UINT64 samp_step = static_cast<UINT64>(in_samrate * 1.0 / out_samrate * 1.0 * one_std);

	double tic = GetTickCount();
	int n_threads = 8;
	int n_samples_per_thread = n_out_samples / n_threads;
	std::vector<std::thread> threads(n_threads);
	for (int i = 0; i < n_threads; ++i)
	{
		threads[i] = std::thread([&, i]() {
			UINT64 pos = one_std + n_samples_per_thread * samp_step * i;
			fun_cubic_16sc(v_samples.data(), v_out_samples.data() + n_samples_per_thread * i, v_coef.data(), pos, samp_step, n_samples_per_thread);
			});
	}

	for (auto& thread : threads)
		thread.join();

	double toc = GetTickCount();
	std::cout << "signal time:" << n_samples * 1000.0 / in_samrate << std::endl;
	std::cout << "Time taken: " << toc - tic << " ms" << std::endl;
	double t_ori = n_samples * 1000.0 / in_samrate;
	double t_task = (toc - tic);
	;
	std::cout << "cubic_16sc_multithread_ipp_int_address:Time ratio: " << t_task / t_ori << std::endl;
};

int main()
{
	cubic_float_address();
	cubic_int_address();
	cubic_ipp_int_address();
	cubic_multithread_ipp_int_address();
	cubic_complex_multithread_ipp_int_address();
	cubic_16sc_multithread_ipp_int_address();
	return 0;
}