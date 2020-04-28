#include <vector>

#include "GNCUtils.hpp"
#include "TestHelpers.hpp"

/**
 * Checks that two Vector3s are approximately equal.
 *
 * @param   kVecA Vector3 A.
 * @param   kVecB Vector3 B.
 */
#define CHECK_VEC3_APPROX(kVecA, kVecB)                                        \
    CHECK_WAPPROX (kVecA.x, kVecB.x);                                          \
    CHECK_WAPPROX (kVecA.y, kVecB.y);                                          \
    CHECK_WAPPROX (kVecA.z, kVecB.z);

/**
 * Checks that two Quaternions are approximately equal.
 *
 * @param   kQuatA Quaternion A.
 * @param   kQuatB Quaternion B.
 */
#define CHECK_QUAT_APPROX(kQuatA, kQuatB)                                      \
    CHECK_WAPPROX (kQuatA.w, kQuatB.w);                                        \
    CHECK_WAPPROX (kQuatA.x, kQuatB.x);                                        \
    CHECK_WAPPROX (kQuatA.y, kQuatB.y);                                        \
    CHECK_WAPPROX (kQuatA.z, kQuatB.z);

/**
 * Configures DV for testing reading and writing GNC objects.
 */
#define INIT_DV                                                                \
    std::shared_ptr<DataVector> pDv = nullptr;                                 \
    CHECK_SUCCESS (DataVector::createNew (gDvConfig, pDv));

/**
 * Inputs and outputs for a vector-quaternion rotation test.
 */
typedef struct
{
    Quaternion quat;  // Quaternion to rotate by.
    Vector3 vec;      // Vector to rotate.
    Vector3 expected; // Expected vector.
} QuatRotTest_t;

/**
 * Test cases for vector-quaternion rotation. Answers sourced from the Eigen
 * linear algebra library: http://eigen.tuxfamily.org/
 */
static std::vector<QuatRotTest_t> gQuatRotTests =
{
    {{ 0.6252, -0.1941,  0.5203,  0.5485}, { 0.8233, -0.6049, -0.3296}, { 0.2751, -0.0651, -1.0356}},
    {{ 0.7594, -0.6292,  0.1528, -0.0640}, { 0.2577, -0.2704,  0.0268}, { 0.2777, -0.1036,  0.2290}},
    {{ 0.6792,  0.6251,  0.2038,  0.3263}, {-0.7168,  0.2139, -0.9674}, {-1.2074,  0.1935, -0.0150}},
    {{-0.4025, -0.5679,  0.4761, -0.5374}, {-0.1981, -0.7404, -0.7824}, { 0.5492,  0.9443, -0.0794}},
    {{ 0.7492, -0.4231,  0.0194,  0.5092}, { 0.2253, -0.4079,  0.2751}, { 0.3157,  0.2977,  0.3233}},
    {{ 0.0470, -0.0124,  0.9146, -0.4014}, { 0.5427,  0.0535,  0.5398}, {-0.4876, -0.3923, -0.4441}},
    {{-0.2072,  0.8129, -0.4499, -0.3063}, { 0.6154,  0.8381, -0.8605}, {-0.2004, -1.3259,  0.1526}},
    {{ 0.6564,  0.0380, -0.6047, -0.4496}, { 0.3265,  0.7805, -0.3022}, { 0.6307,  0.1058,  0.6309}},
    {{-0.5567, -0.6130, -0.0540, -0.5580}, {-0.5234,  0.9413,  0.8044}, {-0.1181, -1.2128,  0.5676}},
    {{ 0.7952, -0.5287,  0.0901, -0.2828}, { 0.5205,  0.0251,  0.3354}, { 0.5861, -0.0116,  0.2011}},
    {{ 0.0497, -0.7252, -0.0982,  0.6797}, { 0.8616,  0.4419, -0.4314}, { 0.5114, -0.2238, -0.9012}},
    {{ 0.6539,  0.3837, -0.4001,  0.5150}, {-0.6681, -0.1198,  0.7602}, {-0.0797, -0.9604, -0.3313}},
    {{ 0.5446, -0.2807, -0.4484,  0.6508}, {-0.2993,  0.3733,  0.9129}, {-0.8755, -0.5429,  0.0331}},
    {{ 0.2183,  0.3874,  0.8833, -0.1488}, { 0.8479, -0.2031,  0.6295}, {-0.4947,  0.1200, -0.9473}},
    {{ 0.3457,  0.7713, -0.0329, -0.5333}, { 0.9005,  0.8403, -0.7047}, { 1.2493, -0.6642, -0.1075}},
    {{ 0.8882,  0.3289, -0.1586,  0.2788}, {-0.4379,  0.5720, -0.3851}, {-0.6528,  0.4472, -0.2025}},
    {{-0.1115, -0.5767, -0.6580, -0.4712}, { 0.1129, -0.1670, -0.6608}, {-0.6003, -0.2089,  0.2707}},
    {{ 0.5980, -0.5833, -0.5496, -0.0067}, { 0.5210,  0.9695,  0.8700}, { 0.2705,  1.2529, -0.5704}},
    {{ 0.5170, -0.3274,  0.7000, -0.3681}, {-0.4117, -0.5355,  0.1690}, { 0.3081,  0.0399,  0.6231}},
    {{-0.4144, -0.5637,  0.3764, -0.6073}, { 0.5869, -0.6718,  0.4901}, { 0.7935, -0.1561,  0.6180}},
};

/**
 * DV config for testing reading and writing GNC objects.
 */
static DataVector::Config_t gDvConfig =
{
    // Region
    {DV_REG_TEST0,

    // Elements
    {
        DV_ADD_FLOAT  (DV_ELEM_TEST0, 0),
        DV_ADD_FLOAT  (DV_ELEM_TEST1, 0),
        DV_ADD_FLOAT  (DV_ELEM_TEST2, 0),
        DV_ADD_FLOAT  (DV_ELEM_TEST3, 0),
        DV_ADD_DOUBLE (DV_ELEM_TEST4, 0),
    }},
};

TEST_GROUP (GNCUtilsTest)
{
};

/**
 * Tests strong floating point approximation.
 */
TEST (GNCUtilsTest, Approx)
{
    // Check approx with a large order of magnitude.
    CHECK_TRUE (GNCUtils::approx ( 1e12,  1e12 + 1));
    CHECK_TRUE (GNCUtils::approx (-1e12, -1e12 - 1));

    CHECK_TRUE (!GNCUtils::approx ( 1e12,  1e12 + 1e6));
    CHECK_TRUE (!GNCUtils::approx (-1e12, -1e12 - 1e6));

    // Check approx with a small order of magnitude.
    CHECK_TRUE (GNCUtils::approx (    M_PI, 3.141592654));
    CHECK_TRUE (GNCUtils::approx (sqrt (2), 1.414213562));

    CHECK_TRUE (!GNCUtils::approx (    M_PI, 3.14159));
    CHECK_TRUE (!GNCUtils::approx (sqrt (2), 1.41421));

    // Check approx with a very small order of magnitude.
    CHECK_TRUE (GNCUtils::approx ( 1e-12,  1e-12 + 1e-24));
    CHECK_TRUE (GNCUtils::approx (-1e-12, -1e-12 - 1e-24));

    CHECK_TRUE (!GNCUtils::approx ( 1e-12,  1e12 + 1e-16));
    CHECK_TRUE (!GNCUtils::approx (-1e-12, -1e12 - 1e-16));

    // Check approx in trivial cases where signs or orders of mag differ.
    CHECK_TRUE (!GNCUtils::approx (1e-6,  1e6));
    CHECK_TRUE (!GNCUtils::approx ( 1e6, -1e6));
}

/**
 * Tests weak floating point approximation.
 */
TEST (GNCUtilsTest, WeakApprox)
{
    CHECK_TRUE (GNCUtils::weakApprox (    M_PI, 3.14159));
    CHECK_TRUE (GNCUtils::weakApprox (sqrt (2), 1.41421));

    Real_t x = 1;
    CHECK_TRUE (GNCUtils::weakApprox (1, x));

    // Push x up against the approximate limit. Take 99% of max negligence to
    // account for FP rounding.
    x += GNCUtils::WEAK_APPROX_EPSILON * 0.99;
    CHECK_TRUE (GNCUtils::weakApprox (1, x));

    // Push x just over the approximate limit. Take 2% of max negligence to
    // account for FP rounding.
    x += GNCUtils::WEAK_APPROX_EPSILON * 0.02;
    CHECK_TRUE (!GNCUtils::weakApprox (1, x));

    // Do the same thing but in the negative direction.
    x = -1 - GNCUtils::WEAK_APPROX_EPSILON * 0.99;
    CHECK_TRUE (GNCUtils::weakApprox (-1, x));

    x -= GNCUtils::WEAK_APPROX_EPSILON * 0.02;
    CHECK_TRUE (!GNCUtils::weakApprox (-1, x));
}

/**
 * Tests Vector3 operations.
 */
TEST (GNCUtilsTest, Vector3Ops)
{
    // Check cross product computation. Answers verified with Wolfram Alpha.
    Vector3 vecA = {1, 2, 3};
    Vector3 vecB = {0, 0, 0};
    Vector3 vecC = vecA.cross (vecB);
    CHECK_EQUAL (0, vecC.x);
    CHECK_EQUAL (0, vecC.y);
    CHECK_EQUAL (0, vecC.z);

    vecA = {-1.5, 0.25,  9.76};
    vecB = {34.6, 8.102, 6.0 };
    vecC = vecA.cross (vecB);
    Vector3 vecD = {-77.57552, 346.696, -20.803};
    CHECK_VEC3_APPROX (vecD, vecC);

    vecA = {   4.0,  0.0, 66.5};
    vecB = {-100.0, 45.0,  9.0};
    vecC = vecA.cross (vecB);
    vecD = {-2992.5, -6686, 180};
    CHECK_VEC3_APPROX (vecD, vecC);

    // Check magnitude computation.
    vecA = {0, 0, 0};
    CHECK_EQUAL (0, vecA.magnitude ());

    vecA = {1, 2, 3};
    CHECK_WAPPROX (3.74165, vecA.magnitude ());

    vecA = {-5, 0.25, 8};
    CHECK_WAPPROX (9.43729, vecA.magnitude ());

    // Check scalar multiplication.
    vecA = {1, 2, 3};
    vecB = vecA * 0;
    CHECK_EQUAL (0, vecB.x);
    CHECK_EQUAL (0, vecB.y);
    CHECK_EQUAL (0, vecB.z);

    vecB = vecA * -2;
    CHECK_EQUAL (-2, vecB.x);
    CHECK_EQUAL (-4, vecB.y);
    CHECK_EQUAL (-6, vecB.z);

    vecB = vecA * 9.81;
    vecC = {9.81, 19.62, 29.43};
    CHECK_VEC3_APPROX (vecC, vecB);

    vecB = vecA * -9.81;
    vecC = {-9.81, -19.62, -29.43};
    CHECK_VEC3_APPROX (vecC, vecB);

    // Check vector addition.
    vecA = {1, 2, 3};
    vecB = {4, 5, 6};
    vecC = vecA + vecB;
    CHECK_EQUAL (5, vecC.x);
    CHECK_EQUAL (7, vecC.y);
    CHECK_EQUAL (9, vecC.z);

    vecA = {1.1,   2.7,  3.0};
    vecB = {0.9, -95.0, 33.3};
    vecC = vecA + vecB;
    vecD = {2.0, -92.3, 36.3};
    CHECK_VEC3_APPROX (vecD, vecC);
}

/**
 * Test reading and writing Vector3s to the Data Vector.
 */
TEST (GNCUtilsTest, Vector3DvReadWrite)
{
    INIT_DV;

    // Write a vector to the DV and read it back out.
    Vector3 vecA = {1, 2, 3};
    CHECK_SUCCESS (GNCUtils::dvWriteVector3 (pDv, vecA, DV_ELEM_TEST0,
                                                        DV_ELEM_TEST1,
                                                        DV_ELEM_TEST2));
    Real_t x = 0;
    Real_t y = 0;
    Real_t z = 0;
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST0, x));
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST1, y));
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST2, z));
    CHECK_EQUAL (vecA.x, x);
    CHECK_EQUAL (vecA.y, y);
    CHECK_EQUAL (vecA.z, z);

    // Read into another vector and compare contents.
    Vector3 vecB = {0, 0, 0};
    CHECK_SUCCESS (GNCUtils::dvReadVector3 (pDv, vecB, DV_ELEM_TEST0,
                                                       DV_ELEM_TEST1,
                                                       DV_ELEM_TEST2));
    CHECK_EQUAL (vecA.x, vecB.x);
    CHECK_EQUAL (vecA.y, vecB.y);
    CHECK_EQUAL (vecA.z, vecB.z);

    // Check errors when providing elems that don't exist or are wrong type.
    CHECK_ERROR (E_DATA_VECTOR_WRITE,
                 GNCUtils::dvWriteVector3 (pDv, vecA, DV_ELEM_TEST0,
                                                      DV_ELEM_TEST1,
                                                      DV_ELEM_TEST5));
    CHECK_ERROR (E_DATA_VECTOR_WRITE,
                 GNCUtils::dvWriteVector3 (pDv, vecA, DV_ELEM_TEST0,
                                                      DV_ELEM_TEST1,
                                                      DV_ELEM_TEST4));
    CHECK_ERROR (E_DATA_VECTOR_READ,
                 GNCUtils::dvReadVector3 (pDv, vecA, DV_ELEM_TEST0,
                                                     DV_ELEM_TEST1,
                                                     DV_ELEM_TEST5));
    CHECK_ERROR (E_DATA_VECTOR_READ,
                 GNCUtils::dvReadVector3 (pDv, vecA, DV_ELEM_TEST0,
                                                     DV_ELEM_TEST1,
                                                     DV_ELEM_TEST4));
}

/**
 * Tests Quaternion normalization.
 */
TEST (GNCUtilsTest, QuaternionNormalization)
{
    // Start with the unit quaternion, which is normalized.
    Quaternion quatA = {1, 0, 0, 0};
    CHECK_TRUE (quatA.isNormalized ());
    
    // Change scalar component by non-negligible amount and verify unnormalized.
    quatA.w += GNCUtils::WEAK_APPROX_EPSILON * 1.01;
    CHECK_TRUE (!quatA.isNormalized ());

    quatA = {1, 1, 1, 1};
    CHECK_TRUE (!quatA.isNormalized ());

    Quaternion quatB = {0.5, 0.5, 0.5, 0.5};
    CHECK_TRUE (quatB.isNormalized ());

    CHECK_SUCCESS (quatA.normalize ());
    CHECK_QUAT_APPROX (quatB, quatA);

    quatA = {4, -3, 0.25, 9.71};
    quatB = {0.36615, -0.27461, 0.02288, 0.88882};
    CHECK_SUCCESS (quatA.normalize ());
    CHECK_QUAT_APPROX (quatA, quatB);

    // Impossible to normalize the zero quaternion.
    quatA = {0, 0, 0, 0};
    CHECK_ERROR (E_NONNORMAL_QUATERNION, quatA.normalize ());
    CHECK_TRUE (!quatA.isNormalized ());
}

/**
 * Tests rotating vectors by quaternions.
 */
TEST (GNCUtilsTest, QuaternionVectorRotation)
{
    // Rotating a vector by the unit quaternion produces the same vector.
    Quaternion quat = {1, 0, 0, 0};
    Vector3 vecA = {1, 2, 3};
    Vector3 vecB = quat.rotate (vecA);
    CHECK_VEC3_APPROX (vecA, vecB);

    // Rotating a vector that lies on the X axis about the X axis produces the
    // same vector.
    quat = {0.707107, 0.707107, 0, 0}; // Roll 90, pitch 0, yaw 0
    vecA = {1, 0, 0};
    vecB = quat.rotate (vecA);
    CHECK_VEC3_APPROX (vecA, vecB);

    // Now try 90 degrees about the Y axis. <1, 0, 0> should become <0, 0, 1>.
    quat = {0, 0.707107, 0, 0.707107}; // Roll 0, pitch 90, yaw 0
    vecB = quat.rotate (vecA);
    Vector3 vecC = {0, 0, 1};
    CHECK_VEC3_APPROX (vecC, vecB);

    // Do some more rigorous tests against Eigen's implementation.
    for (QuatRotTest_t& test : gQuatRotTests)
    {
        Vector3 result = test.quat.rotate (test.vec);
        CHECK_VEC3_APPROX (test.expected, result);
    }
}

/**
 * Test reading and writing Quaternions to the Data Vector.
 */
TEST (GNCUtilsTest, QuaternionDvReadWrite)
{
    INIT_DV;

    // Write a quaternion to the DV and read it back out.
    Quaternion quatA = {1, 2, 3, 4};
    CHECK_SUCCESS (GNCUtils::dvWriteQuaternion (pDv, quatA, DV_ELEM_TEST0,
                                                            DV_ELEM_TEST1,
                                                            DV_ELEM_TEST2,
                                                            DV_ELEM_TEST3));
    Real_t w = 0;
    Real_t x = 0;
    Real_t y = 0;
    Real_t z = 0;
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST0, w));
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST1, x));
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST2, y));
    CHECK_SUCCESS (pDv->read (DV_ELEM_TEST3, z));
    CHECK_EQUAL (quatA.w, w);
    CHECK_EQUAL (quatA.x, x);
    CHECK_EQUAL (quatA.y, y);
    CHECK_EQUAL (quatA.z, z);

    // Read into another quaternion and compare contents.
    Quaternion quatB = {0, 0, 0, 0};
    CHECK_SUCCESS (GNCUtils::dvReadQuaternion (pDv, quatB, DV_ELEM_TEST0,
                                                           DV_ELEM_TEST1,
                                                           DV_ELEM_TEST2,
                                                           DV_ELEM_TEST3));
    CHECK_EQUAL (quatA.w, quatB.w);
    CHECK_EQUAL (quatA.x, quatB.x);
    CHECK_EQUAL (quatA.y, quatB.y);
    CHECK_EQUAL (quatA.z, quatB.z);

    // Check errors when providing elems that don't exist or are wrong type.
    CHECK_ERROR (E_DATA_VECTOR_WRITE,
                 GNCUtils::dvWriteQuaternion (pDv, quatA, DV_ELEM_TEST0,
                                                          DV_ELEM_TEST1,
                                                          DV_ELEM_TEST2,
                                                          DV_ELEM_TEST5));
    CHECK_ERROR (E_DATA_VECTOR_WRITE,
                 GNCUtils::dvWriteQuaternion (pDv, quatA, DV_ELEM_TEST0,
                                                          DV_ELEM_TEST1,
                                                          DV_ELEM_TEST2,
                                                          DV_ELEM_TEST4));
    CHECK_ERROR (E_DATA_VECTOR_READ,
                 GNCUtils::dvReadQuaternion (pDv, quatA, DV_ELEM_TEST0,
                                                         DV_ELEM_TEST1,
                                                         DV_ELEM_TEST2,
                                                         DV_ELEM_TEST5));
    CHECK_ERROR (E_DATA_VECTOR_READ,
                 GNCUtils::dvReadQuaternion (pDv, quatA, DV_ELEM_TEST0,
                                                         DV_ELEM_TEST1,
                                                         DV_ELEM_TEST2,
                                                         DV_ELEM_TEST4));
}