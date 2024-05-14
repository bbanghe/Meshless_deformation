//
//  JR_Picker3D.hpp
//  nanoVGTest
//
//  Created by Hyun Joon Shin on 2/16/24.
//

#ifndef JR_Picker3D_h
#define JR_Picker3D_h
#include <functional>


namespace JGL2 {

	struct pos_t {
		float x, y;
	};

	using glm::vec2;
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	typedef std::function<bool(const vec3&)> Motion3DCallback_t;
	typedef std::function<bool(int)> Button3DCallback_t; // button_t를 int로 변경
	enum class event_t {
		MOVE,
		DRAG,
		RELEASE,
		PUSH,
	};

	struct Picker3D {
		virtual inline vec3		eventPt3D() { return _cursorPt3; }
	
		virtual inline void		move3DCB  (Motion3DCallback_t f) { _move3DCB=f; }
		virtual inline void		unsetMove3DCB() { _move3DCB=defMotion3DCB; }
	
		virtual inline void		drag3DCB  (Motion3DCallback_t f) { _drag3DCB=f; }
		virtual inline void		unsetDrag3DCB() { _drag3DCB=defMotion3DCB; }
	
		virtual inline void		push3DCB  (Button3DCallback_t f) { _push3DCB=f; }
		virtual inline void		unsetPush3DCB() { _push3DCB=defButton3DCB; }
	
		virtual inline void		release3DCB  (Button3DCallback_t f) { _release3DCB=f; }
		virtual inline void		unsetRelease3DCB() { _release3DCB=defButton3DCB; }
	
		bool handle(GLFWwindow* window, event_t e, const glm::ivec2& sz, const mat4& vp, float windowH);
		virtual pos_t			getFramebufferCursorPos(GLFWwindow* window, float windowH);
	
		std::tuple<vec3, float> getNCCursorPos(GLFWwindow* window, const glm::ivec2& sz, float windowH);
		std::tuple<vec3, float> get3DCursorPos(GLFWwindow* window, const glm::ivec2& sz, const mat4& vp, float windowH);

		vec3 getNCCursorPos(GLFWwindow* window, float d, const glm::ivec2& sz, float windowH);
		vec3 get3DCursorPos(GLFWwindow* window, float d, const glm::ivec2& sz, const mat4& vp, float windowH);

		inline void lockCursorDepth() { _cursorDepthLocked = true; }
		inline void releaseCursorDepth() { _cursorDepthLocked = false; }
	
	protected:
		const Motion3DCallback_t defMotion3DCB = [](const vec3&){ return false; };
		const Button3DCallback_t defButton3DCB = [](int){ return false; };
	
		Motion3DCallback_t	_move3DCB = defMotion3DCB;
		Motion3DCallback_t	_drag3DCB = defMotion3DCB;
		Button3DCallback_t	_push3DCB = defButton3DCB;
		Button3DCallback_t	_release3DCB = defButton3DCB;
	
		bool				_cursorDepthLocked = false;
		vec3				_cursorPt3;
		float				_cursorD;
	};


	inline bool Picker3D::handle(GLFWwindow* window, event_t e, const glm::ivec2& sz, const mat4& vp, float windowH) {
		switch( e ) {
			case event_t::MOVE:
				std::tie(_cursorPt3,_cursorD) = get3DCursorPos(window, sz,vp,windowH);
				_move3DCB(_cursorPt3);
				break;
			case event_t::PUSH:
				if(_push3DCB(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))) 
					return true;
				break;
			case event_t::DRAG: {
				vec3 oldPt3 = _cursorPt3;
				if( _cursorDepthLocked )	
					_cursorPt3 = get3DCursorPos(window, _cursorD,sz,vp,windowH);
				else						
					std::tie(_cursorPt3,_cursorD) = get3DCursorPos(window, sz,vp,windowH);
				if( _drag3DCB(_cursorPt3-oldPt3) ) 
					return true;
			}
				break;
			case event_t::RELEASE:
				releaseCursorDepth();
				if (_release3DCB(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))) 
						return true;
				break;
			default:
				break;
		}
		return false;
	}

	//cursor 위치 framebuffer 좌표로 
	inline pos_t Picker3D::getFramebufferCursorPos(GLFWwindow* window, float windowH) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		pos_t pt = { static_cast<float>(xpos), static_cast<float>(ypos) };
		pt.y = windowH - pt.y;

		int fbWidth, fbHeight;
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		int winWidth, winHeight;
		glfwGetWindowSize(window, &winWidth, &winHeight);
		float ratio = static_cast<float>(fbWidth) / static_cast<float>(winWidth);

		return pos_t{ pt.x * ratio, pt.y * ratio };
	}

	//Normalized Coordinate와 depth 값 
	inline std::tuple<vec3,float> Picker3D::getNCCursorPos(GLFWwindow* window, const glm::ivec2& sz, float windowH) {
		pos_t pt = getFramebufferCursorPos(window, windowH);
		float d = 0;
		glReadPixels(static_cast<int>(pt.x), static_cast<int>(pt.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &d);
		glm::vec3 pt3 = glm::vec3(pt.x / (static_cast<float>(sz.x)), pt.y / (static_cast<float>(sz.y)), d) * 2.0f - 1.0f;
		return std::make_tuple(pt3, d);
	}

	inline vec3 Picker3D::getNCCursorPos(GLFWwindow* window, float d, const glm::ivec2& sz, float windowH) {
		pos_t pt = getFramebufferCursorPos(window, windowH);
		glm::vec3 pt3 = glm::vec3(pt.x / (static_cast<float>(sz.x)), pt.y / (static_cast<float>(sz.y)), d) * 2.0f - 1.0f;
		return pt3;
	}

	//Normalized Coordinate	를 WorldCoord로 
	inline std::tuple<vec3,float> Picker3D::get3DCursorPos(GLFWwindow* window, const glm::ivec2& sz, const glm::mat4& vp, float windowH) {
		auto [ptNC, d] = getNCCursorPos(window, sz, windowH);
		glm::vec4 ptWC = glm::inverse(vp) * glm::vec4(ptNC, 1.0f);
		return std::make_tuple(glm::vec3(ptWC) / ptWC.w, d);
	}

	inline vec3 Picker3D::get3DCursorPos(GLFWwindow* window, float d, const glm::ivec2& sz, const glm::mat4& vp, float windowH) {
		glm::vec3 ptNC = getNCCursorPos(window, d, sz, windowH);
		glm::vec4 ptWC = glm::inverse(vp) * glm::vec4(ptNC, 1.0f);
		return glm::vec3(ptWC) / ptWC.w;
	}

} // namespace JRender

#endif /* JR_Picker3D_h */
