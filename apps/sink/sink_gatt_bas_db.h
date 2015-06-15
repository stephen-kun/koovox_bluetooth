#ifdef GATT_SERVER_BAS_ENABLED

primary_service {
    uuid : 0x180F,
    name : "BATTERY_SERVICE",
    characteristic {
        uuid        : 0x2A19,
        name        : "BATTERY_LEVEL",
        flags       : [ FLAG_IRQ ],
        properties  : [ read , notify ],
        value       : 0x0,
        client_config
        {
            name  : "BATTERY_LEVEL_CLIENT_CONFIG",
            flags : [ FLAG_IRQ , FLAG_DYNLEN ]
        }
    }
},

primary_service {
    uuid : 0x180D,
    name : "HEART_RATE_SERVICE",
    characteristic {
       uuid       : 0x2A37,
       name       : "HEART_RATE_MEASUREMENT",
       flags      : [ FLAG_IRQ ],
       properties : [ read,  notify],
       value      : 0x0,
       client_config {
           name  : "HEART_RATE_MEASUREMENT_CLIENT_CONFIG",
           flags : [ FLAG_IRQ , FLAG_DYNLEN ]
       }
    }
},


#ifdef ENABLE_KOOVOX

primary_service {
    uuid : 0x1900,
    name : "KOOVOX_SERVICE",
    characteristic {
       uuid       : 0x2B01,
       name       : "GET_CHARACTERISTIC",
       flags      : [ FLAG_IRQ ],
       properties : [ read,write],
       value      : 0x0
    },
	characteristic {
	   uuid 	  : 0x2B02,
	   name 	  : "SET_CHARACTERISTIC",
	   flags	  : [ FLAG_IRQ ],
	   properties : [ read,write ],
	   value	  : 0x0
	},
	characteristic {
	   uuid 	  : 0x2B10,
	   name 	  : "NOTIFY_CHARACTERISTIC",
	   flags	  : [ FLAG_IRQ ],
	   properties : [ notify ],
	   value	  : 0x0
	}
},


#endif

#endif /* GATT_SERVER_BAS_ENABLED */
