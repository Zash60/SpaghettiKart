#include "port/BfCmd.h"
#include "port/BfBase.h"
#include "port/BfSim.h"
#include "port/BfMutator.h"
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
    // Diagnostic: is the base real driving or all-neutral?
    int nz = 0;
    const BfInput* d = Bf_BaseData();
    for (int i = 0; i < Bf_BaseLen(); i++) {
        if (d[i].stickX != 0 || d[i].stickY != 0 || d[i].button != 0) {
            nz++;
        }
    }
    char buf[192];
    snprintf(buf, sizeof(buf), "BF base: %d frames (%d non-neutral)%s", Bf_BaseLen(), nz,
             on ? " (recording)" : " (ready)");
    if (output) {
        *output = buf;
    }
    SPDLOG_INFO("{}", buf);
    return 0;
}

void Bf_RegisterCommands(void) {
    auto console = Ship::Context::GetRawInstance()->GetConsole();
    if (console->HasCommand("bf_record")) {
        return;
    }
    console->AddCommand("bf_record",
                        { BfRecordCmd, "Record P1 inputs as BF base: bf_record 1|0",
                          { { "on", Ship::ArgumentType::NUMBER } } });
    console->AddCommand(
        "bf_playbase",
        { [](std::shared_ptr<Ship::Console> c, std::vector<std::string> a, std::string* o) -> int32_t {
              (void)c;
              int on = a.size() > 1 ? atoi(a[1].c_str()) : 1;
              if (on) {
                  Bf_PlayBaseStart();
              } else {
                  Bf_PlayBaseStop();
              }
              if (o) {
                  *o = on ? "BF playing base" : "BF stopped";
              }
              return 0;
          },
          "Replay recorded base inputs: bf_playbase 1|0", { { "on", Ship::ArgumentType::NUMBER } } });
    console->AddCommand(
        "bf_start",
        { [](std::shared_ptr<Ship::Console> c, std::vector<std::string> a, std::string* o) -> int32_t {
              (void)c;
              (void)o;
              int window = a.size() > 1 ? atoi(a[1].c_str()) : 30;
              int iters = a.size() > 2 ? atoi(a[2].c_str()) : 200;
              Bf_SearchStart(window, iters);
              return 0;
          },
          "Start BF hill-climb: bf_start <windowFrames> <maxIters>",
          { { "window", Ship::ArgumentType::NUMBER }, { "iters", Ship::ArgumentType::NUMBER } } });
    console->AddCommand("bf_stop",
                        { [](std::shared_ptr<Ship::Console> c, std::vector<std::string> a,
                             std::string* o) -> int32_t {
                              (void)c;
                              (void)a;
                              (void)o;
                              Bf_SearchStop();
                              return 0;
                          },
                          "Stop BF search and restore pre-BF state", {} });
    console->AddCommand(
        "bf_loadresult",
        { [](std::shared_ptr<Ship::Console> c, std::vector<std::string> a, std::string* o) -> int32_t {
              (void)c;
              (void)a;
              int rc = Bf_LoadResult(Bf_ResultPath());
              if (o) {
                  *o = rc == 0 ? "BF result loaded as base" : "BF result NOT found";
              }
              SPDLOG_INFO("{}", (rc == 0) ? "BF result loaded as base" : "BF result NOT found");
              return rc;
          },
          "Load result.txt as the BF base", {} });
    console->AddCommand("bf_exportghost",
                        { [](std::shared_ptr<Ship::Console> c, std::vector<std::string> a,
                             std::string* o) -> int32_t {
                              (void)c;
                              (void)a;
                              int rc = Bf_ExportGhost();
                              if (o) {
                                  *o = rc == 0 ? "BF base exported as session ghost"
                                               : "BF export failed";
                              }
                              return rc;
                          },
                          "Export BF base as the time-trial session ghost", {} });
}
