/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace Platform {
namespace File {

inline void UnsafeOpenUrl(const QString &url) {
	return ::File::internal::UnsafeOpenUrlDefault(url);
}

inline void UnsafeOpenEmailLink(const QString &email) {
	return ::File::internal::UnsafeOpenEmailLinkDefault(email);
}

inline void PostprocessDownloaded(const QString &filepath) {
}

} // namespace File
} // namespace Platform
