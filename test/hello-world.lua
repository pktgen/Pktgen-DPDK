package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

require "Pktgen"
printf("Lua Version      : %s\n", pktgen.info.Lua_Version);
printf("Pktgen Version   : %s\n", pktgen.info.Pktgen_Version);
printf("Pktgen Copyright : %s\n", pktgen.info.Pktgen_Copyright);
printf("Pktgen Authors   : %s\n", pktgen.info.Pktgen_Authors);

printf("\nHello World!!!!\n");
