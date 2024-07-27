#include "RWCom.h"
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::serial_port;
//using boost::asio::deadline_timer;
using boost::asio::buffer;
/*
serial_port::baud_rate 波特率，构造参数为unsigned int
serial_port::parity 奇偶校验，构造参数为serial_port::parity::type，enum类型，可以是none, odd, even。
serial_port::flow_control 流量控制，构造参数为serial_port::flow_control::type，enum类型，可以是none software hardware
serial_port::stop_bits 停止位，构造参数为serial_port::stop_bits::type，enum类型，可以是one onepointfive two
serial_port::character_size 字符大小，构造参数为unsigned int
*/
struct RWCom::RWComCtx
{
	RWCom::RWComCtx():sp(ios)
	{
	}

	boost::asio::io_service ios;
	serial_port sp;
	//deadline_timer timer;
	
	//char buf[4096] = { 0 };
	//size_t ret = 0;
	//boost::mutex mutex;

	//void recv_handler(boost::system::error_code ec,
	//	std::size_t bytes_transferred)
	//{
	//	if (!ec)
	//	{
	//		timer->cancel();
	//	}
	//	ret = bytes_transferred;
	//}
};

RWCom::RWCom()
{
	m_ctx = new RWComCtx();
}

RWCom::~RWCom()
{
	delete m_ctx;
}

bool RWCom::Open(const char* com, int parity, int stopbits, int char_size, int baud_rate)
{
	try
	{
		m_ctx->sp.open(com);
		m_ctx->sp.set_option(serial_port::flow_control(serial_port::flow_control::none));	//流量控制
		m_ctx->sp.set_option(serial_port::parity(serial_port::parity::type(parity)));
		m_ctx->sp.set_option(serial_port::stop_bits(serial_port::stop_bits::type(stopbits)));
		m_ctx->sp.set_option(serial_port::character_size(char_size));
		m_ctx->sp.set_option(serial_port::baud_rate(baud_rate));
		return true;
	}
	catch (const std::exception&e)
	{
		printf("open %s failed: %s\n", com, e.what());
	}
	return false;
}

void RWCom::Close()
{
	try
	{
		if (m_ctx->sp.is_open())
		{
			m_ctx->sp.close();
		}
	}
	catch (const std::exception&e)
	{
		printf("close failed: %s\n", e.what());
	}
}
//
//unsigned int RWCom::ReadData(char* buf, unsigned int size, unsigned int timeout_ms)
//{
//	try
//	{
//		m_ctx->mutex.lock();
//		m_ctx->ret = 0;
//		m_ctx->sp.async_read_some(buffer(buf, size),
//			boost::bind(&RWCom::RWComCtx::recv_handler, m_ctx,
//				boost::asio::placeholders::error,//传送错误码 
//				boost::asio::placeholders::bytes_transferred//传送字节数 
//			));
//		m_ctx->mutex.unlock();
//		m_ctx->timer->expires_from_now(boost::posix_time::millisec(timeout_ms));
//		m_ctx->timer->async_wait(boost::bind(&serial_port::cancel, m_ctx->sp));
//		m_ctx->ios.run();//异步情况下使用run才开始执行 
//		m_ctx->ios.reset();//还原状态 
//		return m_ctx->ret;
//	}
//	catch (const std::exception& e)
//	{
//		printf("read failed: %s\n", e.what());
//	}
//	return 0;
//}

std::string RWCom::ReadLine()
{
	std::string line;
	try
	{
		boost::asio::streambuf buf;
		//m_ctx->mutex.lock();
		boost::asio::read_until(m_ctx->sp, buf, "\n");
		//m_ctx->mutex.unlock();
		m_ctx->ios.run();
		m_ctx->ios.reset();

		std::istream is(&buf);
		std::getline(is, line);		
	}
	catch (const std::exception&e)
	{
		printf("read failed: %s\n", e.what());
	}
	return line;
}
