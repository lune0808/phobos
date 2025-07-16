#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

image::image(std::string_view path)
	: base(nullptr)
{
	stbi_set_flip_vertically_on_load(true);
	base = stbi_load(path.data(), &width, &height, &channels, 0);
}

void image::fini()
{
	stbi_image_free(base);
}

bool image::ok() const
{
	return !!base;
}

texture::texture(image const &img, shader_pipeline const &shader, std::string_view name)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.base);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	location = glGetUniformLocation(shader.id, name.data());
}

void texture::fini()
{
	glDeleteTextures(1, &handle);
}

void texture::bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, handle);
	glUniform1i(location, unit);
}


