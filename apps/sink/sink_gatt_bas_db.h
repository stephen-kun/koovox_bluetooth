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

#endif /* GATT_SERVER_BAS_ENABLED */

#ifdef ENABLE_KOOVOX

primary_service{
	uuid : 0xFEE7,
	name : "WeChat",
	characteristic {
		uuid		: 0xFEC7,
		name		: "WRITE_CHARACTERISTIC",
		flags		: [ FLAG_IRQ],
        properties 	: [ write ],
        value      	: 0x0
	},
	characteristic {
		uuid		: 0xFEC8,
		name		: "INDICATE_CHARACTERISTIC",
		flags		: [ FLAG_IRQ ],
        properties 	: [ indicate],
        value      	: 0x0,
        client_config
		{
			name	: "WECHAR_CLIENT_CONFIG",
			flags	: [ FLAG_IRQ , FLAG_DYNLEN ]
		}
	},
	characteristic {
		uuid		: 0xFEC9,
		name		: "READ_CHARACTERISTIC",
		flags		: [ FLAG_IRQ ],
        properties 	: [ read],
		value		: 0x0
	}	
},

#endif


