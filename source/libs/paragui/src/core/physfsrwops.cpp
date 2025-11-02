/*
 * This code provides a glue layer between PhysicsFS and Simple Directmedia
 *  Layer's (SDL) RWops i/o abstraction.
 *
 * License: this code is public domain. I make no warranty that it is useful,
 *  correct, harmless, or environmentally safe.
 *
 * This particular file may be used however you like, including copying it
 *  verbatim into a closed-source project, exploiting it commercially, and
 *  removing any trace of my name from the source (although I hope you won't
 *  do that). I welcome enhancements and corrections to this file, but I do
 *  not require you to send me patches if you make changes.
 *
 * Unless otherwise stated, the rest of PhysicsFS falls under the GNU Lesser
 *  General Public License: http://www.gnu.org/licenses/lgpl.txt
 *
 * SDL falls under the LGPL, too. You can get SDL at http://www.libsdl.org/
 *
 *  This file was written by Ryan C. Gordon. (icculus@clutteredmind.org).
 */

#include <stdio.h>  /* used for SEEK_SET, SEEK_CUR, SEEK_END ... */
#include "physfsrwops.h"

namespace {

struct PhysfsStreamContext {
	PHYSFS_file* file;
};

SDL_IOStreamInterface BuildPhysfsInterface()
{
	SDL_IOStreamInterface iface;
	SDL_INIT_INTERFACE(&iface);
	return iface;
}

SDL_IOStatus PhysfsErrorStatus()
{
	return SDL_IO_STATUS_ERROR;
}

Sint64 SDLCALL PhysfsSize(void* userdata)
{
	auto* ctx = static_cast<PhysfsStreamContext*>(userdata);
	PHYSFS_sint64 len = PHYSFS_fileLength(ctx->file);
	if (len < 0) {
		SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
		return -1;
	}
	return static_cast<Sint64>(len);
}

Sint64 SDLCALL PhysfsSeek(void* userdata, Sint64 offset, SDL_IOWhence whence)
{
	auto* ctx = static_cast<PhysfsStreamContext*>(userdata);
	PHYSFS_sint64 target = 0;

	switch (whence) {
	case SDL_IO_SEEK_SET:
		target = offset;
		break;
	case SDL_IO_SEEK_CUR: {
		PHYSFS_sint64 current = PHYSFS_tell(ctx->file);
		if (current < 0) {
			SDL_SetError("Can't find position in file: %s", PHYSFS_getLastError());
			return -1;
		}
		target = current + offset;
		break;
	}
	case SDL_IO_SEEK_END: {
		PHYSFS_sint64 len = PHYSFS_fileLength(ctx->file);
		if (len < 0) {
			SDL_SetError("Can't find end of file: %s", PHYSFS_getLastError());
			return -1;
		}
		target = len + offset;
		break;
	}
	default:
		SDL_SetError("Invalid 'whence' parameter.");
		return -1;
	}

	if (target < 0) {
		SDL_SetError("Attempt to seek past start of file.");
		return -1;
	}

	if (!PHYSFS_seek(ctx->file, static_cast<PHYSFS_uint64>(target))) {
		SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
		return -1;
	}

	return target;
}

size_t SDLCALL PhysfsRead(void* userdata, void* ptr, size_t size, SDL_IOStatus* status)
{
	auto* ctx = static_cast<PhysfsStreamContext*>(userdata);
	PHYSFS_sint64 rc = PHYSFS_readBytes(ctx->file, ptr, static_cast<PHYSFS_uint64>(size));
	if (rc < 0) {
		if (status)
			*status = PhysfsErrorStatus();
		SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
		return 0;
	}
	if (rc == 0) {
		if (status)
			*status = PHYSFS_eof(ctx->file) ? SDL_IO_STATUS_EOF : SDL_IO_STATUS_NOT_READY;
	}
	return static_cast<size_t>(rc);
}

size_t SDLCALL PhysfsWrite(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status)
{
	auto* ctx = static_cast<PhysfsStreamContext*>(userdata);
	PHYSFS_sint64 rc = PHYSFS_writeBytes(ctx->file, ptr, static_cast<PHYSFS_uint64>(size));
	if (rc < 0 || static_cast<size_t>(rc) != size) {
		if (status)
			*status = PhysfsErrorStatus();
		SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
		return 0;
	}
	return static_cast<size_t>(rc);
}

bool SDLCALL PhysfsFlush(void* userdata, SDL_IOStatus* status)
{
	auto* ctx = static_cast<PhysfsStreamContext*>(userdata);
	if (!PHYSFS_flush(ctx->file)) {
		if (status)
			*status = PhysfsErrorStatus();
		SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
		return false;
	}
	return true;
}

bool SDLCALL PhysfsClose(void* userdata)
{
	auto* ctx = static_cast<PhysfsStreamContext*>(userdata);
	bool ok = PHYSFS_close(ctx->file) != 0;
	if (!ok)
		SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
	delete ctx;
	return ok;
}

SDL_RWops* CreatePhysfsStream(PHYSFS_file* handle)
{
	if (!handle) {
		SDL_SetError("PhysicsFS error: %s", PHYSFS_getLastError());
		return nullptr;
	}

	auto* ctx = new PhysfsStreamContext{handle};
	SDL_IOStreamInterface iface = BuildPhysfsInterface();
	iface.size = PhysfsSize;
	iface.seek = PhysfsSeek;
	iface.read = PhysfsRead;
	iface.write = PhysfsWrite;
	iface.flush = PhysfsFlush;
	iface.close = PhysfsClose;

	SDL_IOStream* stream = SDL_OpenIO(&iface, ctx);
	if (!stream) {
		delete ctx;
		PHYSFS_close(handle);
	}
	return stream;
}

} // namespace

SDL_RWops *PHYSFSRWOPS_makeRWops(PHYSFS_file *handle) {
	if (handle == NULL) {
		SDL_SetError("NULL pointer passed to PHYSFSRWOPS_makeRWops().");
		return NULL;
	}
	return CreatePhysfsStream(handle);
}

SDL_RWops *PHYSFSRWOPS_openRead(const char *fname) {
	return CreatePhysfsStream(PHYSFS_openRead(fname));
}

SDL_RWops *PHYSFSRWOPS_openWrite(const char *fname) {
	return CreatePhysfsStream(PHYSFS_openWrite(fname));
}

SDL_RWops *PHYSFSRWOPS_openAppend(const char *fname) {
	return CreatePhysfsStream(PHYSFS_openAppend(fname));
}

/* end of physfsrwops.cpp ... */
