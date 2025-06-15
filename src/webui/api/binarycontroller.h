
#pragma once

#include "base/bittorrent/torrent.h"
#include "apicontroller.h"
#include "serialize/serialize_torrent.h"

class BinaryController : public APIController
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(BinaryController)

public:
    using APIController::APIController;

private slots:
    void dumpAction();

};
