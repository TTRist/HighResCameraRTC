// https://qiita.com/vs4sh/items/4a9ce178f1b2fd26ea30

// -*- C++ -*-
/*!
 * @file  HighResCamera.cpp
 * @brief ModuleDescription
 * @date $Date$
 *
 * $Id$
 */

#include "HighResCamera.h"
#include "opencv2/opencv.hpp"
#include "confReader.hpp"


#define WIDTH_HD		1280
#define HEIGHT_HD		720
#define WIDTH_FULLHD	1920
#define HEIGHT_FULLHD	1080
#define WIDTH_4K		3840
#define HEIGHT_4K		2160

using namespace std;
using namespace cv;

VideoCapture cam;

// Module specification
// <rtc-template block="module_spec">
static const char* highrescamera_spec[] =
{
  "implementation_id", "HighResCamera",
  "type_name",         "HighResCamera",
  "description",       "ModuleDescription",
  "version",           "1.0.0",
  "vendor",            "VenderName",
  "category",          "Category",
  "activity_type",     "PERIODIC",
  "kind",              "DataFlowComponent",
  "max_instance",      "1",
  "language",          "C++",
  "lang_type",         "compile",

   "conf.default.focus", "500",
   "conf.__widget__.focus", "text",
   "conf.__constraints__.focus", "0<=x<=1000",
   "conf.__type__.focus", "int",
  ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
HighResCamera::HighResCamera(RTC::Manager* manager)
// <rtc-template block="initializer">
	: RTC::DataFlowComponentBase(manager),
	m_imageOut("image", m_image)

	// </rtc-template>
{
}

/*!
 * @brief destructor
 */
HighResCamera::~HighResCamera()
{
}



RTC::ReturnCode_t HighResCamera::onInitialize()
{
	cout << "Initialized [HighResCamera]\n";

	// Registration: InPort/OutPort/Service
	// <rtc-template block="registration">
	// Set InPort buffers

	// Set OutPort buffer
	addOutPort("image", m_imageOut);

	// Set service provider to Ports

	// Set service consumers to Ports

	// Set CORBA Service Ports

	// </rtc-template>

	// <rtc-template block="bind_config">
	bindParameter("focus", m_focus, "0");
	// </rtc-template>

	return RTC::RTC_OK;
}



double window_h, window_w;
RTC::ReturnCode_t HighResCamera::onActivated(RTC::UniqueId ec_id)
{
	cout << "Activated [HighResCamera]\n";
	// 設定ファイル読み込み
	ifstream infile("./../../../MakedFile/setting.ini");
	while (!infile) printf("\r読み込み失敗　パスが違うかも");
	conf::setMap(conf::config_map, infile, R"(=|\s)");
	infile.close();

	window_h = conf::readMap("WINDOW_H");
	window_w = conf::readMap("WINDOW_W");
	int fps = conf::readMap("FPS");
	int camID = conf::readMap("CamID");
	cam.open(camID);//デバイスのオープン
	if (!cam.isOpened()) {
		cout << "MISS cnct cam\n";
		return RTC::RTC_ERROR;
	}

	//解像度の設定 http://linuberry.blog.fc2.com/blog-entry-13.html
	cout << "<< Select Resolusion >>\n";
	cout << "1.HD\n";
	cout << "2.fullHD\n";
	cout << "3.4K\n";
	int mode;
	cin >> mode;
	if (mode == 1) {
		cam.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH_HD);
		cam.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT_HD);
	}
	else if (mode == 2) {
		cam.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH_FULLHD);
		cam.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT_FULLHD);
	}
	else if (mode == 3) {
		cam.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH_4K);
		cam.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT_4K);
	}
	cam.set(CV_CAP_PROP_FPS, fps);// 30fps

	cout << "s...画像保存　img.png\n";
	cout << "t...ポート送信\n";
	cout << "f...焦点変更\n";
	cout << "l...基準線\n";
	cout << "e...露光調整";

	return RTC::RTC_OK;
}


RTC::ReturnCode_t HighResCamera::onDeactivated(RTC::UniqueId ec_id)
{
	cout << "Deactivated [HighResCamera]\n";
	cam.release();
	destroyAllWindows();
	return RTC::RTC_OK;
}


RTC::ReturnCode_t HighResCamera::onExecute(RTC::UniqueId ec_id)
{
	Mat frame, view;
	static bool line_view = false;
	cam.read(frame);
	resize(frame, view, Size(), window_w / frame.cols, window_h / frame.rows);
	
	const int key = cv::waitKey(1000 / 30); //30fps
	if (key == 's') { // 画像保存
		cout << "Save Image\n";
		cv::imwrite("./../../../img.png", frame);
	}
	else if (key == 't') { // ポート送信
		cout << "Send Data\n";
		m_image.width = frame.cols;
		m_image.height = frame.rows;
		m_image.bpp = frame.channels() * 8;
		int len = m_image.width * m_image.height * frame.channels();
		m_image.pixels.length(len);
		// 画像データをOutPortにコピー
		memcpy((void*)&(m_image.pixels[0]), frame.data, len);

		// 画像データをOutPortから出力
		m_imageOut.write();

	}
	/*
	else if (key == 'a') { // オートフォーカスON
		cout << "Auto Focus ON\n";
		cam.set(CAP_PROP_AUTOFOCUS, 1);
	}
	*/
	else if (key == 'f') { // オートフォーカスOFF.m_focus値依存の固定焦点
		double focusVal;
		cout << "フォーカス値：";
		cin >> focusVal;
		cam.set(CAP_PROP_AUTOFOCUS, false);
		cam.set(CAP_PROP_FOCUS, focusVal);
	}
	else if (key == 'l') {
		cout << "Line ON/OFF\n";
		line_view = !line_view;
	}

	else if (key == 'e') {
		double propVal;
		cout << "露光値：";
		cin >> propVal;
		cam.set(CAP_PROP_AUTO_EXPOSURE, 1); //manual mode
		cam.set(CAP_PROP_EXPOSURE, propVal);
	}


	// 線描画
	if (line_view) {
		Point xAxis_s = {0, view.rows / 2}, xAxis_e = { view.cols, view.rows / 2 };
		Point yAxis_s = {view.cols / 2, 0}, yAxis_e = {view.cols / 2, view.rows };
		line(view, xAxis_s, xAxis_e, { 0, 0, 255 });
		line(view, yAxis_s, yAxis_e, { 0, 255, 0 });
	}

	imshow("Capture", view);//画像を表示．

	return RTC::RTC_OK;
}



extern "C"
{

	void HighResCameraInit(RTC::Manager* manager)
	{
		coil::Properties profile(highrescamera_spec);
		manager->registerFactory(profile,
			RTC::Create<HighResCamera>,
			RTC::Delete<HighResCamera>);
	}

};


/*
RTC::ReturnCode_t HighResCamera::onFinalize()
{
  return RTC::RTC_OK;
}
RTC::ReturnCode_t HighResCamera::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
RTC::ReturnCode_t HighResCamera::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
RTC::ReturnCode_t HighResCamera::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
RTC::ReturnCode_t HighResCamera::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
RTC::ReturnCode_t HighResCamera::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
RTC::ReturnCode_t HighResCamera::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
RTC::ReturnCode_t HighResCamera::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/
