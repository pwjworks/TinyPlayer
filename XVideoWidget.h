#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <mutex>
struct AVFrame;
class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	void Init(int width, int height);
	virtual void Repaint(AVFrame* frame);
	XVideoWidget(QWidget* parent);
	~XVideoWidget();
protected:
	//ˢ����ʾ
	void paintGL();

	//��ʼ��gl
	void initializeGL();

	// ���ڳߴ�仯
	void resizeGL(int width, int height);
private:
	std::mutex mux;

	//shader����
	QGLShaderProgram program;

	//shader��yuv������ַ
	GLuint unis[3] = { 0 };
	//openg�� texture��ַ
	GLuint texs[3] = { 0 };

	//�����ڴ�ռ�
	unsigned char* datas[3] = { 0 };

	int width = 240;
	int height = 128;

};