#pragma once
#include "Vector.hpp"

struct Matrix4f {
	Vector4f rows[4];

	static Matrix4f identity() noexcept {
		Matrix4f m;
		for (size_t i = 0u; i < 4; ++i) {
			m[i][i] = 1;
		}
		return m;
	}

	static Matrix4f diagonal(float scalar) noexcept {
		Matrix4f matrix;
		for (size_t i = 0u; i < 4; ++i) {
			matrix[i][i] = scalar;
		}
		return matrix;
	}
	static Matrix4f scale(float scalar) noexcept {
		Matrix4f matrix;
		for (size_t i = 0u; i + 1 < 4; ++i) {
			matrix[i][i] = scalar;
		}
		matrix[3][3] = 1;
		return matrix;
	}

	static Matrix4f translation(Vector3f vec) noexcept {
		Matrix4f matrix;
		for (size_t i = 0u; i < 3; ++i) {
			matrix[i][4 - 1] = vec[i];
			matrix[i][i] = 1;
		}
		matrix[3][3] = 1;
		return matrix;
	}

	static Matrix4f perspective(float fov, float ratio, float n, float f) noexcept {
		Matrix4f matrix;
		matrix[0] = Vector4f{ (std::cosf(fov / 2) / std::sinf(fov / 2)) / ratio, 0, 0, 0 };
		matrix[1] = Vector4f{ 0, (std::cosf(fov / 2) / std::sinf(fov / 2)) , 0, 0 };
		matrix[2] = Vector4f{ 0, 0, f / (f - n), 1 };
		matrix[3] = Vector4f{ 0, 0, 0, - f * n / (f - n)};
		return matrix;
	}


	static Matrix4f look_at(Vector3f eye, Vector3f center, Vector3f up) noexcept {
		Vector3f f = (center - eye).normalize();
		Vector3f s = up.cross(f).normalize();
		Vector3f u = f.cross(s);

		Matrix4f result = Matrix4f::identity();
		result[0][0] = s.x;
		result[1][0] = s.y;
		result[2][0] = s.z;
		result[0][1] = u.x;
		result[1][1] = u.y;
		result[2][1] = u.z;
		result[0][2] = f.x;
		result[1][2] = f.y;
		result[2][2] = f.z;
		result[3][0] = -(s.dot(eye));
		result[3][1] = -(u.dot(eye));
		result[3][2] = -(f.dot(eye));

		return result;
	}

	static Matrix4f rotation(Vector3f a, float θ) {
		auto c = cosf(θ);
		auto s = sinf(θ);

		auto x = a.x;
		auto y = a.y;
		auto z = a.z;

		float ele[]{
			x * x * (1 - c) + c, x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0.f,
			y * x * (1 - c) + z * s, y * y * (1 - c) + c, y * z * (1 - c) - x * s, 0.f,
			z * x * (1 - c) - y * s, z * x * (1 - c) + x * s, c + z * z * (1 - c), 0.f,
			0.f, 0.f, 0.f, 1.f
		};
		return Matrix4f{ele};
	}

	Matrix4f() noexcept {
		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				rows[i][j] = 0;
			}
		}
	}
	Matrix4f(float ele[16]) noexcept {
		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				rows[i][j] = ele[i + j * 4];
			}
		}
	}

	template<typename U>
	Matrix4f operator+(U scalar) const {
		Matrix4f result;

		for (size_t i = 0u; i < 4; ++i) {
			auto vec = result[i];
			for (size_t j = 0u; j < 4; ++j) {
				vec[j] += scalar;
			}
			result[i] = vec;
		}
		return result;
	}
	Matrix4f operator+(const Matrix4f& other) const {
		Matrix4f result;

		for (size_t i = 0u; i < 4; ++i) {
			auto vec = result[i];
			for (size_t j = 0u; j < 4; ++j) {
				vec[j] += other[i][j];
			}
			result[i] = vec;
		}
		return result;
	}
	template<typename U>
	Matrix4f& operator+=(U scalar) {
		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				rows[i][j] += (float)scalar;
			}
		}
		return *this;
	}
	Matrix4f operator+=(const Matrix4f& other) {
		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				rows[i][j] += (float)other[i][j];
			}
		}
		return *this;
	}

	Matrix4f operator-() const {
		Matrix4f m;

		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				m[i][j] = -rows[i][j];
			}
		}
		return *this;
	}

	template<typename U>
	Matrix4f operator-(U scalar) const {
		return *this + (-scalar);
	}
	Matrix4f operator-(const Matrix4f& other) const {
		return *this + (-other);
	}
	template<typename U>
	Matrix4f& operator-=(U scalar) {
		return *this += (-scalar);
	}
	Matrix4f operator-=(const Matrix4f& other) {
		return *this += (-other);
	}

	template<typename U>
	Matrix4f operator*(U scalar) const {
		Matrix4f result;

		for (size_t i = 0u; i < 4; ++i) {
			auto vec = result[i];
			for (size_t j = 0u; j < 4; ++j) {
				vec[j] *= (float)scalar;
			}
			result[i] = vec;
		}
		return result;
	}
	template<typename U>
	Matrix4f& operator*=(U scalar) {
		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				rows[i][j] *= (float)scalar;
			}
		}
		return *this;
	}
	
	Vector<4, float> operator*(Vector<4, float> vec) const {
		Vector<4, float> result;

		for (size_t i = 0u; i < 4; ++i) {
			float sum = (float)0;
			for (size_t j = 0u; j < 4; ++j) {
				sum += vec[j] * rows[i][j];
			}
			result[i] = sum;
		}
		return result;
	}

	Matrix4f operator*(const Matrix4f& other) const {
		Matrix4f matrix;

		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				float sum = (float)0;

				for (size_t n = 0u; n < 4; ++n) {
					sum += rows[i][n] * other.rows[n][j];
				}

				matrix[i][j] = sum;
			}
		}

		return matrix;
	}
	Matrix4f& operator*=(const Matrix4f& other) {
		for (size_t i = 0u; i < 4; ++i) {
			for (size_t j = 0u; j < 4; ++j) {
				float sum = (float)0;

				for (size_t n = 0u; n < 4; ++n) {
					sum += rows[i][n] * other.rows[n][j];
				}

				rows[i][j] = sum;
			}
		}

		return *this;
	}

	template<typename U>
	Matrix4f operator/(U scalar) const {
		return *this * (1 / scalar);
	}
	template<typename U>
	Matrix4f& operator/=(U scalar) {
		return *this *= (1 / scalar);
	}

	Vector<4, float> operator[](size_t i) const {
		return rows[i];
	}
	Vector<4, float>& operator[](size_t i) {
		return rows[i];
	}

	float operator[](const Vector<2, unsigned int>& idx) const {
		return rows[idx.x][idx.y];
	}
	float& operator[](const Vector<2, unsigned int>& idx) {
		return rows[idx.x][idx.y];
	}

	Matrix4f to_col() noexcept {
		Matrix4f result;
		for (size_t i = 0; i < 4; ++i) {
			for (size_t j = 0; j < 4; ++j) {
				result[{j, i}] = this->operator[]({i, j});
			}
		}
		return result;
	}

};

