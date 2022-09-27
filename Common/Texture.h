#include <string>

class Texture
{
private:
	unsigned int m_texture_id;
	std::string m_file_path;
	unsigned char* m_local_buffer;
	int m_width, m_height, m_bytes_per_pixel;
public:
	Texture(const std::string& path);
	~Texture();

	void bind(unsigned int slot_number = 0) const;
	void unbind() const;

	inline int width() { return m_width; };
	inline int height() { return m_height; };
};