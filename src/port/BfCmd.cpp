#include "port/BfCmd.h"
#include "port/BfBase.h"
#include <libultraship.h>
#include <spdlog/spdlog.h>

static int32_t BfRecordCmd(std::shared_ptr<Ship::Console> console, std::vector<std::string> args,
                           std::string* output) {
    (void)console;
    int on = args.size() > 1 ? atoi(args[1].c_str()) : 1;
    if (on) {
        Bf_RecordStart();
    } else {
        Bf_RecordStop();
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "BF base: %d frames%s", Bf_BaseLen(), on ? " (recording)" : " (ready)");
    if (output) {
        *output = buf;
    }
    SPDLOG_INFO("{}", buf);
    return 0;
}

void Bf_RegisterCommands(void) {
    auto console = Ship::Context::GetInstance()->GetConsole();
    if (console->HasCommand("bf_record")) {
        return;
    }
    console->AddCommand("bf_record",
                        { BfRecordCmd, "Record P1 inputs as BF base: bf_record 1|0",
                          { { "on", Ship::ArgumentType::NUMBER } } });
}
