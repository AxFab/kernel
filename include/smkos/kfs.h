#pragma once

#include <smkos/kapi.h>
#include <smkos/klimits.h>
#include <smkos/compiler.h>
#include <smkos/kstruct/fs.h>
#include <smkos/kstruct/map.h>
#include <smkos/arch.h>
#include <smkos/drivers.h>


typedef kDevice_t device_t;

void *dev_map(device_t* dev, size_t lba, size_t cnt);
void dev_unmap(void* add);
void dev_sync(device_t *dev);
