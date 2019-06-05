#pragma warning(push)
#pragma warning(disable: 26495)
#pragma warning(disable: 6001)
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <openvr.h>
#pragma warning(pop)

#include <string>
#include <iostream>

glm::mat4 VR2GL(const vr::HmdMatrix44_t& mat);
glm::mat4 VR2GL(const vr::HmdMatrix34_t& mat);
void VR2GL(const vr::HmdMatrix34_t& mat, glm::vec3& position, glm::quat& rotation, bool invert = false);

template<typename T>
inline void WriteStream(std::ostream& stream, T var) {
	stream.write(reinterpret_cast<const char*>(&var), sizeof(T));
}
template<>
inline void WriteStream<std::string>(std::ostream& stream, std::string var) {
	WriteStream(stream, (uint32_t)var.length());
	stream.write(var.c_str(), var.length() + 1);
}

template<typename T>
inline T ReadStream(std::istream& stream) {
	T r;
	stream.read(reinterpret_cast<char*>(&r), sizeof(T));
	return r;
}
template<>
inline std::string ReadStream<std::string>(std::istream& stream) {
	size_t sz = (size_t)ReadStream<uint32_t>(stream);
	char* s = new char[sz + (size_t)1];
	stream.read(s, (sz + (size_t)1));
	std::string str = s;
	delete[] s;
	return str;
}