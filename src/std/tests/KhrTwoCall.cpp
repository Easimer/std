#include "std/KhrTwoCall.hpp"
#include "std/Testing.hpp"

enum FoofResult {
  FOOF_SUCCESS = 0,
  FOOF_INCOMPLETE = 1,
  FOOF_ERROR_VALIDATION_ERROR = -1,
  FOOF_ERROR_SILLY = -2,
};

struct FoofDeviceDescriptor {
  int numLegs;
};

struct FoofGetDeviceInfo {
  int numLegsWanted;
};

typedef struct FoofInstace_T *FoofInstance;

static FoofResult foofEnumerateDevices(FoofInstance instance,
                                       FoofGetDeviceInfo *info,
                                       uint32_t *pDeviceCount,
                                       FoofDeviceDescriptor *pDevices) {
  if (pDeviceCount == nullptr || (*pDeviceCount != 0 && pDevices == nullptr)) {
    return FOOF_ERROR_VALIDATION_ERROR;
  }

  if (info->numLegsWanted < 4) {
    return FOOF_ERROR_SILLY;
  }

  const int NUM_DEVICES = 3;
  if (pDevices == nullptr || *pDeviceCount < NUM_DEVICES) {
    *pDeviceCount = NUM_DEVICES;
    return FOOF_INCOMPLETE;
  }

  *pDeviceCount = NUM_DEVICES;
  for (int i = 0; i < NUM_DEVICES; i++) {
    pDevices[i] = {
        .numLegs = info->numLegsWanted + i,
    };
  }

  return FOOF_SUCCESS;
}

SN_TEST(KhrTwoCall, basic) {
  Arena::Scope temp;
  FoofResult rc;

  MutSlice<FoofDeviceDescriptor> devices;
  FoofInstance instance = nullptr;

  FoofGetDeviceInfo info = {
      .numLegsWanted = 4,
  };

  CHECK(devices.empty());
  uint32_t numCallCount = 0;

  rc = twoCall<FoofResult>(temp, devices,
                           [instance, &info, &numCallCount](
                               u32 &numElems, FoofDeviceDescriptor *arr) {
                             numCallCount++;
                             return foofEnumerateDevices(instance, &info,
                                                         &numElems, arr);
                           });

  CHECK(rc == FOOF_SUCCESS);
  CHECK(numCallCount == 2);
  CHECK(devices.length != 0);
  CHECK(devices[0].numLegs == info.numLegsWanted);
}

SN_TEST(KhrTwoCall, error) {
  Arena::Scope temp;
  FoofResult rc;

  MutSlice<FoofDeviceDescriptor> devices;
  FoofInstance instance = nullptr;

  FoofGetDeviceInfo info = {
      .numLegsWanted = 0,
  };

  CHECK(devices.empty());
  uint32_t numCallCount = 0;

  rc = twoCall<FoofResult>(temp, devices,
                           [instance, &info, &numCallCount](
                               u32 &numElems, FoofDeviceDescriptor *arr) {
                             numCallCount++;
                             return foofEnumerateDevices(instance, &info,
                                                         &numElems, arr);
                           });

  CHECK(rc == FOOF_ERROR_SILLY);
  CHECK(numCallCount == 1);
  CHECK(devices.empty());
}

SN_TEST(KhrTwoCall, errorAfterAlloc) {
  Arena::Scope temp;
  FoofResult rc;

  MutSlice<FoofDeviceDescriptor> devices;
  FoofInstance instance = nullptr;

  FoofGetDeviceInfo info = {
      .numLegsWanted = 4,
  };

  CHECK(devices.empty());
  uint32_t numCallCount = 0;

  rc = twoCall<FoofResult>(temp, devices,
                           [instance, &info, &numCallCount](
                               u32 &numElems, FoofDeviceDescriptor *arr) {
                             numCallCount++;
                             FoofResult rc = foofEnumerateDevices(instance, &info,
                                                         &numElems, arr);

                             info.numLegsWanted = 0;

                             return rc;
                           });

  CHECK(rc == FOOF_ERROR_SILLY);
  CHECK(numCallCount == 2);
  // No allocations were made
  CHECK(devices.empty());
  // Arena is in the original state
  CHECK(temp.arena->beg == temp.saved.beg);
  CHECK(temp.arena->end == temp.saved.end);
}

SN_TEST(KhrTwoCallV, basic) {
  Arena::Scope temp;

  MutSlice<FoofDeviceDescriptor> devices;
  FoofInstance instance = nullptr;

  FoofGetDeviceInfo info = {
      .numLegsWanted = 4,
  };

  CHECK(devices.empty());
  uint32_t numCallCount = 0;

  twoCallV(temp, devices,
           [instance, &info, &numCallCount](u32 &numElems,
                                            FoofDeviceDescriptor *arr) {
             numCallCount++;
             foofEnumerateDevices(instance, &info, &numElems, arr);
           });

  CHECK(numCallCount == 2);
  CHECK(devices.length != 0);
  CHECK(devices[0].numLegs == info.numLegsWanted);
}
