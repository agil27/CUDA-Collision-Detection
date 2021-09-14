#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

using namespace glm;

// 定义摄像机移动的动作
enum Movement {
	FORWARD, BACKWARD, LEFTWARD, RIGHTWARD
};

// 默认摄相机参数
const vec3 POS = vec3(0.0f, 0.0f, 0.0f);
const vec3 UP = vec3(0.0f, 1.0f, 0.0f);
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// 摄像机类
class Camera
{
public:
	// 摄像机向量
	vec3 _pos, _front, _up, _right, _worldup;
	// 欧拉角
	float _yaw, _pitch;
	// 摄像机选项
	float _speed, _sensitivity, _zoom;

	// 构造函数
	Camera(vec3 pos = POS, vec3 up = UP, float yaw = YAW, float pitch = PITCH) :
		_speed(SPEED), _sensitivity(SENSITIVITY), _zoom(ZOOM)
	{
		_pos = pos;
		_worldup = up;
		_yaw = yaw;
		_pitch = pitch;
		updateCameraVectors();
	}

	// 获取观察矩阵
	mat4 getViewMatrix()
	{
		return lookAt(_pos, _pos + _front, _up);
	}

	// 处理键盘指令，用于摄像机位移
	void processKey(Movement m, float dt)
	{
		float v = _speed * dt;
		switch (m)
		{
		case FORWARD:
			_pos += _front * v;
			break;
		case BACKWARD:
			_pos -= _front * v;
			break;
		case LEFTWARD:
			_pos -= _right * v;
			break;
		case RIGHTWARD:
			_pos += _right * v;
			break;
		}
	}

	// 处理鼠标移动，用于旋转视角
	void processMouseMovement(float offsetX, float offsetY, GLboolean constraintPitch=true)
	{
		_yaw += offsetX * _sensitivity;
		_pitch += offsetY * _sensitivity;
		if (constraintPitch) {
			if (_pitch > 89.0f) {
				_pitch = 89.0f;
			}
			if (_pitch < -89.0f) {
				_pitch = -89.0f;
			}
		}
		updateCameraVectors();
	}

	// 处理鼠标滚轮，用于缩放视角
	void processMouseScroll(float offset)
	{
		_zoom -= offset;
		if (_zoom < 1.0f) {
			_zoom = 1.0f;
		}
		if (_zoom > 45.0f) {
			_zoom = 45.0f;
		}
	}

	float getZoom() {
		return _zoom;
	}
	
private:
	void updateCameraVectors() {
		_front.x = cos(radians(_yaw)) * cos(radians(_pitch));
		_front.y = sin(radians(_pitch));
		_front.z = sin(radians(_yaw)) * cos(radians(_pitch));
		_front = normalize(_front);
		_right = normalize(cross(_front, _worldup));
		_up = normalize(cross(_right, _front));
	}
};
#endif