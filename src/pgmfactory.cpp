#include "pgmfactory.h"
#include "pic16a.h"
#include "pic16b.h"

std::shared_ptr<IDeviceProgrammer> ProgrammerFactory::create(const std::string &deviceFamily, std::shared_ptr<Serial> serial)
{
    if (deviceFamily == "CF_P16F_A")
    {
        return std::make_shared<PIC16A>(serial);
    }
    else if (deviceFamily == "CF_P16F_B")
    {
        return std::make_shared<PIC16B>(serial);
    }

    return nullptr;
}

