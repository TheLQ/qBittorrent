
#include "binarycontroller.h"

#include <QList>
#include <QByteArray>

#include "base/global.h"
#include "base/logger.h"
#include "base/bittorrent/session.h"
// #include "base/utils/string.h"
#include "serialize/serialize_torrent.h"

void BinaryController::dumpAction() {
    const auto *session = BitTorrent::Session::instance();

    QByteArray result;
    for (const BitTorrent::Torrent *torrent : asConst(session->torrents()))
    {
        auto output = serialize_binary(*torrent, QList<QString>());
        // serialize(*torrent, QList)
    }

    LogMsg(QObject::tr("wrote %1 bytes to output").arg(result.length()));

    setResult(result, QStringLiteral("application/octet-stream"), QStringLiteral("raw.dat"));
}
