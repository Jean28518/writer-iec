/*
 * tls_client_exmaple.c
 *
 * This example shows how to configure TLS
 */
#include "iec61850_client.h"

#include <stdlib.h>
#include <stdio.h>

#include "hal_thread.h"


using namespace std;

static TLSConfiguration tlsConfig;

static IedConnection con;

void
reportCallbackFunction(void* parameter, ClientReport report)
{
    MmsValue* dataSetValues = ClientReport_getDataSetValues(report);

    printf("received report for %s\n", ClientReport_getRcbReference(report));

    int i;
    for (i = 0; i < 4; i++) {
        ReasonForInclusion reason = ClientReport_getReasonForInclusion(report, i);

        if (reason != IEC61850_REASON_NOT_INCLUDED) {
            printf("  GGIO1.SPCSO%i.stVal: %i (included for reason %i)\n", i,
                    MmsValue_getBoolean(MmsValue_getElement(dataSetValues, i)), reason);
        }
    }
}

static void initTLSConfig() {
    tlsConfig = TLSConfiguration_create();

    TLSConfiguration_setChainValidation(tlsConfig, true);
    TLSConfiguration_setAllowOnlyKnownCertificates(tlsConfig, false);

    if (!TLSConfiguration_setOwnKeyFromFile(tlsConfig, "client1-key.pem", NULL)) {
        printf("ERROR: Failed to load private key!\n");
        exit(EXIT_FAILURE);
    }

    if (!TLSConfiguration_setOwnCertificateFromFile(tlsConfig, "client1.cer")) {
        printf("ERROR: Failed to load own certificate!\n");
        exit(EXIT_FAILURE);
    }

    if (!TLSConfiguration_addCACertificateFromFile(tlsConfig, "root.cer")) {
        printf("ERROR: Failed to load root certificate\n");
        exit(EXIT_FAILURE);
    }
}

static IedConnection initConnection(char* hostname = (char*) "localhost") {
      IedClientError error;
      con = IedConnection_createWithTlsSupport(tlsConfig);
      IedConnection_connect(con, &error, hostname, -1);
      if (error != IED_ERROR_OK) {
          printf("Failed to connect to %s  %i\n", hostname, error);
          exit(EXIT_FAILURE);
      }
      return con;
}


int main(int argc, char** argv) {
    initTLSConfig();
    if (argc > 1)
        con = initConnection(argv[1]);
    else
        con = initConnection();

    if (con != NULL) {
        IedClientError error;
        LinkedList serverDirectory = IedConnection_getServerDirectory(con, &error, false);

        if (error != IED_ERROR_OK)
            printf("failed to read server directory (error=%i)\n", error);

        if (serverDirectory)
            LinkedList_destroy(serverDirectory);

        /* read an analog measurement value from server */
        MmsValue* value = IedConnection_readObject(con, &error, "simpleIOGenericIO/GGIO1.AnIn1.mag.f", IEC61850_FC_MX);

        if (value != NULL) {
            float fval = MmsValue_toFloat(value);
            printf("read float value: %f\n", fval);
            MmsValue_delete(value);
        }

        /* write a variable to the server */
        value = MmsValue_newVisibleString("libiec61850.com");
        IedConnection_writeObject(con, &error, "simpleIOGenericIO/GGIO1.NamPlt.vendor", IEC61850_FC_DC, value);

        if (error != IED_ERROR_OK)
            printf("failed to write simpleIOGenericIO/GGIO1.NamPlt.vendor!\n");

        MmsValue_delete(value);


        /* read data set */
        ClientDataSet clientDataSet = IedConnection_readDataSetValues(con, &error, "simpleIOGenericIO/LLN0.Events", NULL);

        if (clientDataSet == NULL)
            printf("failed to read dataset\n");

        /* Read RCB values */
        ClientReportControlBlock rcb =
                IedConnection_getRCBValues(con, &error, "simpleIOGenericIO/LLN0.RP.EventsRCB01", NULL);


        bool rptEna = ClientReportControlBlock_getRptEna(rcb);

        printf("RptEna = %i\n", rptEna);

        /* Install handler for reports */
        IedConnection_installReportHandler(con, "simpleIOGenericIO/LLN0.RP.EventsRCB01",
                ClientReportControlBlock_getRptId(rcb), reportCallbackFunction, NULL);

        /* Set trigger options and enable report */
        ClientReportControlBlock_setTrgOps(rcb, TRG_OPT_DATA_UPDATE | TRG_OPT_INTEGRITY | TRG_OPT_GI);
        ClientReportControlBlock_setRptEna(rcb, true);
        ClientReportControlBlock_setIntgPd(rcb, 5000);
        IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_RPT_ENA | RCB_ELEMENT_TRG_OPS | RCB_ELEMENT_INTG_PD, true);

        if (error != IED_ERROR_OK)
            printf("report activation failed (code: %i)\n", error);

        Thread_sleep(1000);

        /* trigger GI report */
        ClientReportControlBlock_setGI(rcb, true);
        IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_GI, true);

        if (error != IED_ERROR_OK)
            printf("Error triggering a GI report (code: %i)\n", error);

        Thread_sleep(60000);

        /* disable reporting */
        ClientReportControlBlock_setRptEna(rcb, false);
        IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_RPT_ENA, true);

        if (error != IED_ERROR_OK)
            printf("disable reporting failed (code: %i)\n", error);

        ClientDataSet_destroy(clientDataSet);

        ClientReportControlBlock_destroy(rcb);

        close_connection:

        IedConnection_close(con);
    }

    IedConnection_destroy(con);

    TLSConfiguration_destroy(tlsConfig);
    return 0;
}
