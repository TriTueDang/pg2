#include "Texture.hpp"
#include <iostream>

GLuint Texture::gen_ckboard(void) {
    if (ckboard_ == 0 || glIsTexture(ckboard_) != GL_TRUE) {
        glCreateTextures(GL_TEXTURE_2D, 1, &ckboard_);

        cv::Vec3b black{ 0, 0, 0 };
        cv::Vec3b white{ 255, 255, 255 };
        cv::Mat ckb = cv::Mat(2, 2, CV_8UC3, black);
        ckb.at<cv::Vec3b>(0, 0) = white;
        ckb.at<cv::Vec3b>(1, 1) = white;

        glTextureStorage2D(ckboard_, 1, GL_RGB8, ckb.cols, ckb.rows);
        glTextureSubImage2D(ckboard_, 0, 0, 0, ckb.cols, ckb.rows, GL_BGR, GL_UNSIGNED_BYTE, ckb.data);
        glTextureParameteri(ckboard_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(ckboard_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(ckboard_, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(ckboard_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    return ckboard_;
}

cv::Mat Texture::load_image(const std::filesystem::path& path) {
    cv::Mat image = cv::imread(path.string(), cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        throw std::runtime_error{ std::string("no texture in file: ").append(path.string()) };
    }
    return image;
}

Texture::Texture(cv::Mat const& image, Interpolation interpolation, bool flip)
{
    if (image.empty()) {
        throw std::runtime_error{ "the input image is empty" };
    }

    cv::Mat processed;
    if (flip) {
        cv::flip(image, processed, 0);
    } else {
        processed = image;
    }

    if (processed.depth() != CV_8U) {
        processed.convertTo(processed, CV_8U, processed.depth() == CV_16U ? 1.0 / 256.0 : 1.0);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &name_);

    switch (processed.type()) {
    case CV_8UC1:
        glTextureStorage2D(name_, 1, GL_R8, processed.cols, processed.rows);
        glTextureSubImage2D(name_, 0, 0, 0, processed.cols, processed.rows, GL_RED, GL_UNSIGNED_BYTE, processed.data);
        glTextureParameteri(name_, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTextureParameteri(name_, GL_TEXTURE_SWIZZLE_B, GL_RED);
        break;
    case CV_8UC3:
        glTextureStorage2D(name_, 1, GL_RGB8, processed.cols, processed.rows);
        glTextureSubImage2D(name_, 0, 0, 0, processed.cols, processed.rows, GL_BGR, GL_UNSIGNED_BYTE, processed.data);
        break;
    case CV_8UC4:
        glTextureStorage2D(name_, 1, GL_RGBA8, processed.cols, processed.rows);
        glTextureSubImage2D(name_, 0, 0, 0, processed.cols, processed.rows, GL_BGRA, GL_UNSIGNED_BYTE, processed.data);
        break;
    default:
        throw std::runtime_error{ "unsupported number of channels or channel depth in texture" };
    }

    set_interpolation(interpolation);

    glTextureParameteri(name_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(name_, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(const glm::vec3 & vec) : Texture{ cv::Mat{1, 1, CV_8UC3, cv::Scalar{vec.b * 255.0f, vec.g * 255.0f, vec.r * 255.0f}}, Interpolation::nearest, false } {}

Texture::Texture(const glm::vec4 & vec) : Texture{ cv::Mat{1, 1, CV_8UC4, cv::Scalar{vec.b * 255.0f, vec.g * 255.0f, vec.r * 255.0f, vec.a * 255.0f}}, Interpolation::nearest, false } {}

Texture::Texture(const std::filesystem::path & path, Interpolation interpolation, bool flip) : Texture{ load_image(path), interpolation, flip } {}

Texture::~Texture() {
    if (name_ != 0 && name_ != ckboard_) {
        glDeleteTextures(1, &name_);
    }
}

GLuint Texture::get_name() const {
    return name_ != 0 ? name_ : gen_ckboard();
}

void Texture::bind(GLuint unit) const {
    glBindTextureUnit(unit, get_name());
}

void Texture::set_interpolation(Interpolation interpolation) {
    switch (interpolation) {
    case Interpolation::nearest:
        glTextureParameteri(name_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(name_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    case Interpolation::linear:
        glTextureParameteri(name_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(name_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case Interpolation::linear_mipmap_linear:
        glTextureParameteri(name_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(name_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateTextureMipmap(name_);
        break;
    }
}

int Texture::get_height(void) const {
    int tex_height = 0;
    glGetTextureLevelParameteriv(get_name(), 0, GL_TEXTURE_HEIGHT, &tex_height);
    return tex_height;
}

int Texture::get_width(void) const {
    int tex_width = 0;
    glGetTextureLevelParameteriv(get_name(), 0, GL_TEXTURE_WIDTH, &tex_width);
    return tex_width;
}

void Texture::replace_image(const cv::Mat& image) {
    if ((image.rows != get_height()) || (image.cols != get_width()))
        throw std::runtime_error("improper image replacement size");

    int tex_format = 0;
    glGetTextureLevelParameteriv(name_, 0, GL_TEXTURE_INTERNAL_FORMAT, &tex_format);

    switch (image.type()) {
    case CV_8UC1:
        if (tex_format != GL_R8) throw std::runtime_error("improper image format");
        glTextureSubImage2D(name_, 0, 0, 0, image.cols, image.rows, GL_RED, GL_UNSIGNED_BYTE, image.data);
        break;
    case CV_8UC3:
        if (tex_format != GL_RGB8) throw std::runtime_error("improper image format");
        glTextureSubImage2D(name_, 0, 0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.data);
        break;
    case CV_8UC4:
        if (tex_format != GL_RGBA8) throw std::runtime_error("improper image format");
        glTextureSubImage2D(name_, 0, 0, 0, image.cols, image.rows, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
        break;
    default:
        throw std::runtime_error{ "unsupported number of channels" };
    }
}
