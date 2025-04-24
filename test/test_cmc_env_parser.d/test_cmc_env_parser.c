

void __test_parse_full_kea_env_file(void) {
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  struct cmc_ConfigField *dhcp4, *ifcfg, *ifaces, *iface0;
  struct cmc_ConfigField *lease, *type, *persist, *name;
  struct cmc_ConfigField *subnets, *subnet, *subnet_val;
  struct cmc_ConfigField *pools, *pool_dict, *pool_val;
  struct cmc_ConfigField *opts, *opt0, *opt0_name, *opt0_data;
  struct cmc_ConfigField *opt1, *opt1_name, *opt1_data;
  struct cmc_ConfigField *lifetime;

  // Build config field hierarchy
  cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &dhcp4);
  cmc_config_add_field(dhcp4, config);

  cmc_field_create("interfaces_config", cmc_ConfigFieldTypeEnum_DICT, NULL,
                   true, &ifcfg);
  cmc_field_add_nested_field(dhcp4, ifcfg);

  cmc_field_create("interfaces", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                   &ifaces);
  cmc_field_add_nested_field(ifcfg, ifaces);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &iface0);
  cmc_field_add_nested_field(ifaces, iface0);

  cmc_field_create("lease_database", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                   &lease);
  cmc_field_add_next_field(ifcfg, lease);

  cmc_field_create("type", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &type);
  cmc_field_add_nested_field(lease, type);

  cmc_field_create("persist", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &persist);
  cmc_field_add_next_field(type, persist);

  cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &name);
  cmc_field_add_next_field(persist, name);

  cmc_field_create("subnet4", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                   &subnets);
  cmc_field_add_next_field(lease, subnets);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &subnet);
  cmc_field_add_nested_field(subnets, subnet);

  cmc_field_create("subnet", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &subnet_val);
  cmc_field_add_nested_field(subnet, subnet_val);

  cmc_field_create("pools", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true, &pools);
  cmc_field_add_next_field(subnet_val, pools);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &pool_dict);
  cmc_field_add_nested_field(pools, pool_dict);

  cmc_field_create("pool", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &pool_val);
  cmc_field_add_nested_field(pool_dict, pool_val);

  cmc_field_create("option_data", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                   &opts);
  cmc_field_add_next_field(pools, opts);

  cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &opt0);
  cmc_field_add_nested_field(opts, opt0);

  cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &opt0_name);
  cmc_field_add_nested_field(opt0, opt0_name);

  cmc_field_create("data", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                   &opt0_data);
  cmc_field_add_next_field(opt0_name, opt0_data);

  cmc_field_create("valid_lifetime", cmc_ConfigFieldTypeEnum_INT, NULL, true,
                   &lifetime);
  cmc_field_add_next_field(subnets, lifetime);

  // Parse using the ENV parser directly
  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *s = NULL;
  int v = -1;

  cmc_field_get_str(iface0, &s);
  TEST_ASSERT_EQUAL_STRING("eth0", s);

  cmc_field_get_str(type, &s);
  TEST_ASSERT_EQUAL_STRING("memfile", s);

  cmc_field_get_str(persist, &s);
  TEST_ASSERT_EQUAL_STRING("true", s);

  cmc_field_get_str(name, &s);
  TEST_ASSERT_EQUAL_STRING("/var/lib/kea/dhcp4.leases", s);

  cmc_field_get_str(subnet_val, &s);
  TEST_ASSERT_EQUAL_STRING("192.168.1.0/24", s);

  cmc_field_get_str(pool_val, &s);
  TEST_ASSERT_EQUAL_STRING("192.168.1.100 - 192.168.1.200", s);

  cmc_field_get_str(opt0_name, &s);
  TEST_ASSERT_EQUAL_STRING("routers", s);
  cmc_field_get_str(opt0_data, &s);
  TEST_ASSERT_EQUAL_STRING("192.168.1.1", s);

  opt1 = opt0->next_field;
  TEST_ASSERT_NOT_NULL(opt1);
  opt1_name = opt1->value;
  TEST_ASSERT_NOT_NULL(opt1_name);
  opt1_data = opt1_name->next_field;
  TEST_ASSERT_NOT_NULL(opt1_data);

  cmc_field_get_str(opt1_name, &s);
  TEST_ASSERT_EQUAL_STRING("domain-name-servers", s);
  cmc_field_get_str(opt1_data, &s);
  TEST_ASSERT_EQUAL_STRING("8.8.8.8", s);

  cmc_field_get_int(lifetime, &v);
  TEST_ASSERT_EQUAL_INT(3600, v);
}
