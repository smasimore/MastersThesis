#include "GNCUtils.hpp"

/********************************* VECTOR3 ************************************/

Vector3 Vector3::cross (const Vector3& kRhs) const
{
    return {y * kRhs.z - z * kRhs.y,
            z * kRhs.x - x * kRhs.z,
            x * kRhs.y - y * kRhs.x};
}

Real_t Vector3::magnitude () const
{
    return sqrt (x * x + y * y + z * z);
}

Vector3 Vector3::operator+ (const Vector3& kRhs) const
{
    return {x + kRhs.x, y + kRhs.y, z + kRhs.z};
}

/******************************** QUATERNION **********************************/

Vector3 Quaternion::rotate (const Vector3& kVec) const
{
    // One of the more computationally inexpensive vector rotation formulas.
    // See section "Vector Rotation" of
    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    Vector3 q = {x, y, z};
    Vector3 t = q.cross (kVec) * 2.0;
    return kVec + t * w + q.cross (t);
}

Real_t Quaternion::magnitude () const
{
    return sqrt (w * w + x * x + y * y + z * z);
}

Error_t Quaternion::normalize ()
{
    Real_t mag = this->magnitude ();
    w /= mag;
    x /= mag;
    y /= mag;
    z /= mag;

    return this->isNormalized () ? E_SUCCESS : E_NONNORMAL_QUATERNION;
}

bool Quaternion::isNormalized () const
{
    return GNCUtils::approx (this->magnitude (), 1);
}

/********************************* UTILITIES **********************************/

const Real_t GNCUtils::WEAK_APPROX_EPSILON = 1e-3;

Error_t GNCUtils::dvReadVector3 (std::shared_ptr<DataVector>& kPDv,
                                 Vector3& kVec,
                                 DataVectorElement_t kXElem,
                                 DataVectorElement_t kYElem,
                                 DataVectorElement_t kZElem)
{
    if (kPDv->read (kXElem, kVec.x) != E_SUCCESS ||
        kPDv->read (kYElem, kVec.y) != E_SUCCESS ||
        kPDv->read (kZElem, kVec.z) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    return E_SUCCESS;
}

Error_t GNCUtils::dvWriteVector3 (std::shared_ptr<DataVector>& kPDv,
                                  const Vector3& kVec,
                                  DataVectorElement_t kXElem,
                                  DataVectorElement_t kYElem,
                                  DataVectorElement_t kZElem)
{
    if (kPDv->write (kXElem, kVec.x) != E_SUCCESS ||
        kPDv->write (kYElem, kVec.y) != E_SUCCESS ||
        kPDv->write (kZElem, kVec.z) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}

Error_t GNCUtils::dvReadQuaternion (std::shared_ptr<DataVector>& kPDv,
                                    Quaternion& kQuat,
                                    DataVectorElement_t kWElem,
                                    DataVectorElement_t kXElem,
                                    DataVectorElement_t kYElem,
                                    DataVectorElement_t kZElem)
{
    if (kPDv->read (kWElem, kQuat.w) != E_SUCCESS ||
        kPDv->read (kXElem, kQuat.x) != E_SUCCESS ||
        kPDv->read (kYElem, kQuat.y) != E_SUCCESS ||
        kPDv->read (kZElem, kQuat.z) != E_SUCCESS)
    {
        return E_DATA_VECTOR_READ;
    }

    return E_SUCCESS;
}

Error_t GNCUtils::dvWriteQuaternion (std::shared_ptr<DataVector>& kPDv,
                                     const Quaternion& kQuat,
                                     DataVectorElement_t kWElem,
                                     DataVectorElement_t kXElem,
                                     DataVectorElement_t kYElem,
                                     DataVectorElement_t kZElem)
{
    if (kPDv->write (kWElem, kQuat.w) != E_SUCCESS ||
        kPDv->write (kXElem, kQuat.x) != E_SUCCESS ||
        kPDv->write (kYElem, kQuat.y) != E_SUCCESS ||
        kPDv->write (kZElem, kQuat.z) != E_SUCCESS)
    {
        return E_DATA_VECTOR_WRITE;
    }

    return E_SUCCESS;
}