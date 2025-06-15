/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2018-2025  Vladimir Golovnev <glassez@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "serialize_torrent.h"

#include <QDateTime>
#include <QList>

#include "base/bittorrent/infohash.h"
#include "base/bittorrent/torrent.h"
#include "base/bittorrent/trackerentrystatus.h"
#include "base/path.h"
#include "base/tagset.h"
#include "base/utils/datetime.h"
#include "base/utils/string.h"

namespace
{
    QString torrentStateToString(const BitTorrent::TorrentState state)
    {
        switch (state)
        {
        case BitTorrent::TorrentState::Error:
            return u"error"_s;
        case BitTorrent::TorrentState::MissingFiles:
            return u"missingFiles"_s;
        case BitTorrent::TorrentState::Uploading:
            return u"uploading"_s;
        case BitTorrent::TorrentState::StoppedUploading:
            return u"stoppedUP"_s;
        case BitTorrent::TorrentState::QueuedUploading:
            return u"queuedUP"_s;
        case BitTorrent::TorrentState::StalledUploading:
            return u"stalledUP"_s;
        case BitTorrent::TorrentState::CheckingUploading:
            return u"checkingUP"_s;
        case BitTorrent::TorrentState::ForcedUploading:
            return u"forcedUP"_s;
        case BitTorrent::TorrentState::Downloading:
            return u"downloading"_s;
        case BitTorrent::TorrentState::DownloadingMetadata:
            return u"metaDL"_s;
        case BitTorrent::TorrentState::ForcedDownloadingMetadata:
            return u"forcedMetaDL"_s;
        case BitTorrent::TorrentState::StoppedDownloading:
            return u"stoppedDL"_s;
        case BitTorrent::TorrentState::QueuedDownloading:
            return u"queuedDL"_s;
        case BitTorrent::TorrentState::StalledDownloading:
            return u"stalledDL"_s;
        case BitTorrent::TorrentState::CheckingDownloading:
            return u"checkingDL"_s;
        case BitTorrent::TorrentState::ForcedDownloading:
            return u"forcedDL"_s;
        case BitTorrent::TorrentState::CheckingResumeData:
            return u"checkingResumeData"_s;
        case BitTorrent::TorrentState::Moving:
            return u"moving"_s;
        default:
            return u"unknown"_s;
        }
    }
}

QVariantMap serialize(const BitTorrent::Torrent &torrent, const QList<QString> &fields)
{
    const auto adjustQueuePosition = [](const int position) -> int
    {
        return (position < 0) ? 0 : (position + 1);
    };

    const auto adjustRatio = [](const qreal ratio) -> qreal
    {
        return (ratio >= BitTorrent::Torrent::MAX_RATIO) ? -1 : ratio;
    };

    const auto getLastActivityTime = [&torrent]() -> qlonglong
    {
        const qlonglong timeSinceActivity = torrent.timeSinceActivity();
        return (timeSinceActivity < 0)
            ? Utils::DateTime::toSecsSinceEpoch(torrent.addedTime())
            : (QDateTime::currentSecsSinceEpoch() - timeSinceActivity);
    };

    const bool hasMetadata = torrent.hasMetadata();

    QVariantMap result;

    #define INSERT_M(key, value) if (fields.isEmpty() || fields.contains(key)) { result.insert(key, value); }

    INSERT_M(KEY_TORRENT_ID, torrent.id().toString());
    INSERT_M(KEY_TORRENT_INFOHASHV1, torrent.infoHash().v1().toString());
    INSERT_M(KEY_TORRENT_INFOHASHV2, torrent.infoHash().v2().toString());
    INSERT_M(KEY_TORRENT_NAME, torrent.name());

    INSERT_M(KEY_TORRENT_HAS_METADATA, hasMetadata);
    INSERT_M(KEY_TORRENT_CREATED_BY, torrent.creator());
    INSERT_M(KEY_TORRENT_CREATION_DATE, Utils::DateTime::toSecsSinceEpoch(torrent.creationDate()));
    INSERT_M(KEY_TORRENT_PRIVATE, (hasMetadata ? torrent.isPrivate() : QVariant()));
    INSERT_M(KEY_TORRENT_TOTAL_SIZE, torrent.totalSize());
    INSERT_M(KEY_TORRENT_PIECES_NUM, torrent.piecesCount());
    INSERT_M(KEY_TORRENT_PIECE_SIZE, torrent.pieceLength());

    INSERT_M(KEY_TORRENT_MAGNET_URI, torrent.createMagnetURI());
    INSERT_M(KEY_TORRENT_SIZE, torrent.wantedSize());
    INSERT_M(KEY_TORRENT_PROGRESS, torrent.progress());
    INSERT_M(KEY_TORRENT_TOTAL_WASTED, torrent.wastedSize());
    INSERT_M(KEY_TORRENT_PIECES_HAVE, torrent.piecesHave());
    INSERT_M(KEY_TORRENT_DLSPEED, torrent.downloadPayloadRate());
    INSERT_M(KEY_TORRENT_UPSPEED, torrent.uploadPayloadRate());
    INSERT_M(KEY_TORRENT_QUEUE_POSITION, adjustQueuePosition(torrent.queuePosition()));
    INSERT_M(KEY_TORRENT_SEEDS, torrent.seedsCount());
    INSERT_M(KEY_TORRENT_NUM_COMPLETE, torrent.totalSeedsCount());
    INSERT_M(KEY_TORRENT_LEECHS, torrent.leechsCount());
    INSERT_M(KEY_TORRENT_NUM_INCOMPLETE, torrent.totalLeechersCount());

    INSERT_M(KEY_TORRENT_STATE, torrentStateToString(torrent.state()));
    INSERT_M(KEY_TORRENT_ETA, torrent.eta());
    INSERT_M(KEY_TORRENT_SEQUENTIAL_DOWNLOAD, torrent.isSequentialDownload());
    INSERT_M(KEY_TORRENT_FIRST_LAST_PIECE_PRIO, torrent.hasFirstLastPiecePriority());

    INSERT_M(KEY_TORRENT_CATEGORY, torrent.category());
    INSERT_M(KEY_TORRENT_TAGS, Utils::String::joinIntoString(torrent.tags(), u", "_s));
    INSERT_M(KEY_TORRENT_SUPER_SEEDING, torrent.superSeeding());
    INSERT_M(KEY_TORRENT_FORCE_START, torrent.isForced());
    INSERT_M(KEY_TORRENT_SAVE_PATH, torrent.savePath().toString());
    INSERT_M(KEY_TORRENT_DOWNLOAD_PATH, torrent.downloadPath().toString());
    INSERT_M(KEY_TORRENT_CONTENT_PATH, torrent.contentPath().toString());
    INSERT_M(KEY_TORRENT_ROOT_PATH, torrent.rootPath().toString());
    INSERT_M(KEY_TORRENT_ADDED_ON, Utils::DateTime::toSecsSinceEpoch(torrent.addedTime()));
    INSERT_M(KEY_TORRENT_COMPLETION_ON, Utils::DateTime::toSecsSinceEpoch(torrent.completedTime()));
    INSERT_M(KEY_TORRENT_TRACKER, torrent.currentTracker());
    INSERT_M(KEY_TORRENT_TRACKERS_COUNT, torrent.trackers().size());
    INSERT_M(KEY_TORRENT_DL_LIMIT, torrent.downloadLimit());
    INSERT_M(KEY_TORRENT_UP_LIMIT, torrent.uploadLimit());
    INSERT_M(KEY_TORRENT_AMOUNT_DOWNLOADED, torrent.totalDownload());
    INSERT_M(KEY_TORRENT_AMOUNT_UPLOADED, torrent.totalUpload());
    INSERT_M(KEY_TORRENT_AMOUNT_DOWNLOADED_SESSION, torrent.totalPayloadDownload());
    INSERT_M(KEY_TORRENT_AMOUNT_UPLOADED_SESSION, torrent.totalPayloadUpload());
    INSERT_M(KEY_TORRENT_AMOUNT_LEFT, torrent.remainingSize());
    INSERT_M(KEY_TORRENT_AMOUNT_COMPLETED, torrent.completedSize());
    INSERT_M(KEY_TORRENT_CONNECTIONS_COUNT, torrent.connectionsCount());
    INSERT_M(KEY_TORRENT_CONNECTIONS_LIMIT, torrent.connectionsLimit());
    INSERT_M(KEY_TORRENT_MAX_RATIO, torrent.maxRatio());
    INSERT_M(KEY_TORRENT_MAX_SEEDING_TIME, torrent.maxSeedingTime());
    INSERT_M(KEY_TORRENT_MAX_INACTIVE_SEEDING_TIME, torrent.maxInactiveSeedingTime());
    INSERT_M(KEY_TORRENT_RATIO, adjustRatio(torrent.realRatio()));
    INSERT_M(KEY_TORRENT_RATIO_LIMIT, torrent.ratioLimit());
    INSERT_M(KEY_TORRENT_POPULARITY, torrent.popularity());
    INSERT_M(KEY_TORRENT_SEEDING_TIME_LIMIT, torrent.seedingTimeLimit());
    INSERT_M(KEY_TORRENT_INACTIVE_SEEDING_TIME_LIMIT, torrent.inactiveSeedingTimeLimit());
    INSERT_M(KEY_TORRENT_LAST_SEEN_COMPLETE_TIME, Utils::DateTime::toSecsSinceEpoch(torrent.lastSeenComplete()));
    INSERT_M(KEY_TORRENT_AUTO_TORRENT_MANAGEMENT, torrent.isAutoTMMEnabled());
    INSERT_M(KEY_TORRENT_TIME_ACTIVE, torrent.activeTime());
    INSERT_M(KEY_TORRENT_SEEDING_TIME, torrent.finishedTime());
    INSERT_M(KEY_TORRENT_LAST_ACTIVITY_TIME, getLastActivityTime());
    INSERT_M(KEY_TORRENT_AVAILABILITY, torrent.distributedCopies());
    INSERT_M(KEY_TORRENT_REANNOUNCE, torrent.nextAnnounce());
    INSERT_M(KEY_TORRENT_COMMENT, torrent.comment())
    return result;
}
