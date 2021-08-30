Basic Ethernet
==============

Port of ESP IDF Basic ethernet example.

Goal is to develop universal ethernet support for all architectures, where hardware support exists.


.. note:

    Currently don't have a PHY attached.
    Default is to receive RMII clock, presumably from PHY, so esp_eth_driver_install() fails with::

        E (130) emac_esp32: emac_esp32_init(354): reset timeout
        E (130) esp_eth: esp_eth_driver_install(204): init mac failed

    Setting CONFIG_ETH_RMII_CLK_OUTPUT=y and rebuilding SDK changes error to:

        E (656) esp_eth.phy: No PHY device detected
        E (656) lan8720: lan8720_init(416): Detect PHY address failed
        E (656) esp_eth: esp_eth_driver_install(205): init phy failed

    When the PHY is connected things should work.


    If a different PHY is used configuration will require changing.
    We need a way to store SDK configuration within a project.
    Having it buried in the framework is not good enough.

    It will be necessary to enable projects to contain IDF SDK settings.
    These can be used to produce a variant for the esp32 Component.

    These values could be prefixed with `ESP32_CONFIG_` prefix so the esp32 Component
    can include these in the generated sdkconfig.

