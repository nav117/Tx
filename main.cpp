#include "radio.h"

#if defined(SX127x_H)
    #define BW_KHZ              125
    #define SPREADING_FACTOR    12
    #define CF_HZ               868300000
    #define TX_DBM              20
#elif defined(SX126x_H)
    #define BW_KHZ              125
    #define SPREADING_FACTOR    7
    #define CF_HZ               868300000
    #define TX_DBM              (Radio::chipType == CHIP_TYPE_SX1262 ? 20 : 14) 
#elif defined(SX128x_H)
    #define BW_KHZ              200
    #define SPREADING_FACTOR    7
    #define CF_HZ               2487000000
    #define TX_DBM              6
#endif

/**********************************************************************/
EventQueue queue(4 * EVENTS_EVENT_SIZE);

void tx_test()
{
    uint8_t cnt = 0;
    
    for(;;){
    Radio::radio.tx_buf[0] = cnt++;  /* set payload */
    Radio::radio.tx_buf[1] = 06;
    Radio::Send(2, 0, 0, 0);   /* begin transmission */
    printf("sent\r\n");
    rtos:ThisThread::sleep_for(2000);
    printf("\r\nCOUNT TO: %d\r",cnt);
    }

/*    {
        mbed_stats_cpu_t stats;
        mbed_stats_cpu_get(&stats);
        printf("canDeep:%u ", sleep_manager_can_deep_sleep());
        printf("Uptime: %llu ", stats.uptime / 1000);
        printf("Sleep time: %llu ", stats.sleep_time / 1000);
        printf("Deep Sleep: %llu\r\n", stats.deep_sleep_time / 1000);
    }*/
}

void txDoneCB()
{
    printf("got-tx-done\r\n");
    queue.call_in(500, tx_test);
}

void rxDoneCB(uint8_t size, float rssi, float snr)
{
}


void radio_irq_callback()
{
    queue.call(Radio::service);
}


const RadioEvents_t rev = {
    /* DioPin_top_half */     radio_irq_callback,
    /* TxDone_topHalf */    NULL,
    /* TxDone_botHalf */    txDoneCB,
    /* TxTimeout  */        NULL,
    /* RxDone  */           rxDoneCB,
    /* RxTimeout  */        NULL,
    /* RxError  */          NULL,
    /* FhssChangeChannel  */NULL,
    /* CadDone  */          NULL
};

int main()
{
    //uint8_t seq = 0;
    uint8_t cnt = 0;

    printf("\r\n starting reset-tx ");
    Radio::radio.tx_buf[0] = cnt;

    Radio::Init(&rev);

    Radio::Standby();
    Radio::LoRaModemConfig(BW_KHZ, SPREADING_FACTOR, 1);
    Radio::SetChannel(CF_HZ);

    Radio::set_tx_dbm(TX_DBM);

               // preambleLen, fixLen, crcOn, invIQ
    Radio::LoRaPacketConfig(8, false, true, false);

    queue.call_in(500, tx_test); //500 è il delay prima che parte il burst
    tx_test();
    //queue.dispatch();
    
}

