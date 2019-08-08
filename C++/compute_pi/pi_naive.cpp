#include <iostream>
#include <math.h>
#include <thread>
#include <random>
#include <algorithm>
#include <iomanip>

using Floating= double;
using Integral= long;
constexpr Integral DEFAULT_NUM_TRY=1000;


const auto parseCommandLineArgs(const int argc,  char** argv){
	class commandLineArgumentException : public std::exception{};
	if(argc!=3)
		throw new commandLineArgumentException();
	
	std::pair<int, int> result = {std::stoi(argv[1]),std::stoi(argv[2])};
	return result;
}



inline bool isInCircle(const Floating& x, const Floating& y, const Floating radius=1.0){
	return (x*x +  y*y <= radius);
}


class pi_worker  {
public:
	Integral id;
	Integral result;
	Integral num_try;
	pi_worker(const Integral _id, const Integral _num_try) : id(_id), num_try(_num_try), result(0)
	{}
	
	void operator()(){
		thread_function();
	}
	 void thread_function(){
	 	//std::cout<<"Thread "<<id<<": Started"<<std::endl;
		const Floating range_from  = 0.0;
		const Floating range_to    = 1.0;
		std::random_device  rand_dev;
		const auto v = rand_dev();
		//std::cout<<"t"<<id<<" "<<v<<std::endl;
		std::mt19937  generator(v);
		std::uniform_real_distribution<Floating>  distr(range_from, range_to);

		for(Integral i = 0; i < num_try ; ++i){
			if(isInCircle(distr(generator),distr(generator)))
				++result;
		}
		//std::cout<<"Thread "<<id<<": End = "<<result<<std::endl;
	}
};



int main(int argc, char** argv){
	const auto parameters = parseCommandLineArgs(argc, argv);
	Integral num_try = parameters.first;
	Integral num_threads = parameters.second;
	using pi_thread = std::pair<std::thread,pi_worker>;
	std::vector<pi_worker> workers;
	workers.reserve(num_threads);
	std::vector<std::thread> threads;
	threads.reserve(num_threads);
	threads.reserve(num_threads);
	for(int i=0;i<num_threads;++i){
		workers.push_back(pi_worker(i,num_try/num_threads));
		threads.push_back(std::thread(std::ref(workers.back())));
	}

std::cout<<"Main Waiting for workers to finish"<<std::endl;
	for(auto& t : threads)
		t.join();

	
	const Integral total_good_try = std::accumulate(workers.begin(), workers.end(), 0,
		[](Integral sum, const pi_worker& worker){
			//std::cout<<worker.result;
			return sum+worker.result;
		}
	);
	const Integral total_num_points = num_threads*(num_try/num_threads);
	const Floating result = 4.0*(static_cast<Floating>(total_good_try*1.0)/static_cast<Floating>(total_num_points*1.0));
	std::cout<<total_good_try<<" "<<total_num_points<<std::endl;
	std::cout<<"Pi~= "<<std::setprecision(15)<<result<<" error = "<<fabs(M_PI-result)<<std::endl;
	printf("%.15f\n", result);

	return 0;
}

