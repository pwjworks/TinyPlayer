#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <mutex>
#include <memory>
#include "IVideoCall.h"
struct AVFrame;
class AVFrameRAII;
class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IVideoCall
{
	Q_OBJECT

public:
	void Init(int width, int height);
	virtual void Repaint(std::shared_ptr<AVFrameRAII> frame);
	XVideoWidget(QWidget* parent);
	~XVideoWidget();
protected:
	//刷新显示
	void paintGL();

	//初始化gl
	void initializeGL();

	// 窗口尺寸变化
	void resizeGL(int width, int height);
private:
	std::mutex mux;

	//shader程序
	QGLShaderProgram program;

	//shader中yuv变量地址
	GLuint unis[3] = { 0 };
	//openg的 texture地址
	GLuint texs[3] = { 0 };

	//材质内存空间
	unsigned char* datas[3] = { 0 };

	int width = 240;
	int height = 128;

};