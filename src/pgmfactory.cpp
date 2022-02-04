#include "pgmfactory.h"
#include "pic16a.h"
#include "pic16b.h"
#include "pic16c.h"

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

    // Note: CF_P16F_C uses a very different command set!
    //       it needs support from the Arduino code.
    // see: 40001753B.pdf
    //
    //else if (deviceFamily == "CF_P16F_C")
    //{
    //    return std::make_shared<PIC16C>(serial);
    //}

    // At first glance, CF_P16F_D has the same command set
    // as CF_P16F_A., see 40001738D.pdf

    return nullptr;
}

