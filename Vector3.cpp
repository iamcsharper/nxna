#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix.h"

namespace Nxna
{
	void Vector3::Normalize()
	{
		float len = Length();

		X /= len;
		Y /= len;
		Z /= len;
	}

	void Vector3::Cross(const Vector3& v1, const Vector3& v2, Vector3& result)
	{
		result.X = v1.Y * v2.Z - v1.Z * v2.Y;
		result.Y = v1.Z * v2.X - v1.X * v2.Z;
		result.Z = v1.X * v2.Y - v1.Y * v2.X;
	}

	 void Vector3::Normalize(const Vector3& v, Vector3& result)
	 {
		 float len = v.Length();

		 result.X = v.X / len;
		 result.Y = v.Y / len;
		 result.Z = v.Z / len;
	 }

	 void Vector3::Transform(const Vector3& v, const Matrix& matrix, Vector3& result)
	 {
		 float x = v.X * matrix.M11 + v.Y * matrix.M21 + v.Z * matrix.M31 + matrix.M41;
         float y = v.X * matrix.M12 + v.Y * matrix.M22 + v.Z * matrix.M32 + matrix.M42;
         float z = v.X * matrix.M13 + v.Y * matrix.M23 + v.Z * matrix.M33 + matrix.M43;

		 result.X = x;
		 result.Y = y;
		 result.Z = z;
	 }

	 void Vector3::Transform(const Vector3& v, const Nxna::Quaternion& quat, Vector3& result)
	 {
		 Quaternion conjugate(-quat.X, -quat.Y, -quat.Z, quat.W);

		 Quaternion qResult1, qResult2;
		 Quaternion::Multiply(quat, Quaternion(v.X, v.Y, v.Z, 0), qResult1);
		 Quaternion::Multiply(qResult1, conjugate, qResult2);

		 result.X = qResult2.X;
		 result.Y = qResult2.Y;
		 result.Z = qResult2.Z;
	 }

	 float Vector3::DistanceSquared(const Vector3& v1, const Vector3& v2)
	 {
		 return (v1.X - v2.X) * (v1.X - v2.X) + (v1.Y - v2.Y) * (v1.Y - v2.Y) + (v1.Z - v2.Z) * (v1.Z - v2.Z);
	 }
}