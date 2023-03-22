#ifndef PHONE_VERSION_H
#define PHONE_VERSION_H

#ifdef __cplusplus
extern "C"
{
#endif

unsigned phone_version_major();
unsigned phone_version_minor();
unsigned phone_version_patch();
__attribute__((unused)) unsigned phone_version_tweak();

#ifdef __cplusplus
}
#endif

#endif //PHONE_VERSION_H
