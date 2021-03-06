#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <iocsh.h>
#include <dbAccess.h>
#include <iocInit.h>

#include "error.h"
#include "epics_device.h"
#include "epics_extra.h"
#include "persistence.h"
#include "pvlogging.h"

#include "example_pvs.h"


extern int example_ioc_registerRecordDeviceDriver(struct dbBase *pdb);

static const char *persistence_file;
static int persistence_interval;


static error__t load_database(const char *db)
{
    database_add_macro("DEVICE", "TS-TS-TEST-99");
    return database_load_file(db);
}


static error__t ioc_main(void)
{
    return
        initialise_epics_device()  ?:

        initialise_example_pvs()  ?:
        start_caRepeater()  ?:
        hook_pv_logging("db/access.acf", 10)  ?:
        load_persistent_state(persistence_file, persistence_interval, false)  ?:

        /* The following block of code could equivalently be implemented by
         * writing a startup script with the following content with a call to
         * iocsh():
         *
         *  dbLoadDatabase("dbd/example_ioc.dbd", NULL, NULL)
         *  example_ioc_registerRecordDeviceDriver(pdbbase)
         *  dbLoadRecords("db/example_ioc.db", "DEVICE=TS-TS-TEST-99")
         *  iocInit()
         */
        TEST_IO(dbLoadDatabase("dbd/example_ioc.dbd", NULL, NULL))  ?:
        TEST_IO(example_ioc_registerRecordDeviceDriver(pdbbase))  ?:
        load_database("db/example_ioc.db")  ?:
        TEST_OK(iocInit() == 0);
}


static error__t parse_args(int argc, const char *argv[])
{
    return
        TEST_OK_(argc == 3, "Wrong number of arguments")  ?:
        DO(
            persistence_file = argv[1];
            persistence_interval = atoi(argv[2]);
        );
}


int main(int argc, const char *argv[])
{
    error__t error =
        parse_args(argc, argv)  ?:
        ioc_main()  ?:
        TEST_IO(iocsh(NULL))  ?:
        DO(terminate_persistent_state());
    return error_report(error) ? 1 : 0;
}
