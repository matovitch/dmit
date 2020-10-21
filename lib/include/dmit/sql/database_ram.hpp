#pragma once

#include "dmit/sql/canary_buffer.hpp"
#include "dmit/sql/connexion.hpp"

namespace dmit::sql
{

class DatabaseRAM
{

public:

    DatabaseRAM(CanaryBuffer& canaryBuffer);

    static void initVfs();

private:

    Connexion _connexion;

    static bool _isVfsInitialized;
};

} // namespace dmit::sql
