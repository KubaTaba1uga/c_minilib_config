#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unity.h>

#include "c_minilib_config.h"
#include "cmc_parse_interface/cmc_env_parser/cmc_env_parser.h"
#include "utils/cmc_error.h"
#include "utils/cmc_field.h"

#ifndef KEA_CONFIG_PATH
#define KEA_CONFIG_PATH "non_exsistent_path"
#endif

static struct cmc_ConfigParseInterface parser;
static struct cmc_Config *config = NULL;
static cmc_error_t err = NULL;

void setUp(void) {
  err = cmc_env_parser_init(&parser);
  TEST_ASSERT_NULL(err);
  config = NULL;
}

void tearDown(void) {
  cmc_config_destroy(&config);
  cmc_error_destroy(&err);
}

void test_kea_interfaces_config_parsing(void) {
  struct cmc_ConfigField *dhcp4, *cfg, *arr, *elem;

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &dhcp4);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(dhcp4, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("interfaces_config", cmc_ConfigFieldTypeEnum_DICT,
                         NULL, true, &cfg);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, cfg);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("interfaces", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &arr);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(cfg, arr);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &elem);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(arr, elem);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  TEST_ASSERT_EQUAL_UINT32(1, arr->_self.subnodes_len);
  struct cmc_ConfigField *iface = cmc_field_of_node(arr->_self.subnodes[0]);
  char *out = NULL;
  err = cmc_field_get_str(iface, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("eth0", out);
}

void test_kea_lease_database_type_parsing(void) {
  struct cmc_ConfigField *dhcp4, *lease, *type;

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &dhcp4);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(dhcp4, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("lease_database", cmc_ConfigFieldTypeEnum_DICT, NULL,
                         true, &lease);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, lease);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("type", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &type);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(lease, type);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *out = NULL;
  err = cmc_field_get_str(type, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("memfile", out);
}

void test_kea_lease_database_name_parsing(void) {
  struct cmc_ConfigField *dhcp4, *lease, *name;

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &dhcp4);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(dhcp4, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("lease_database", cmc_ConfigFieldTypeEnum_DICT, NULL,
                         true, &lease);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, lease);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(lease, name);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *out = NULL;
  err = cmc_field_get_str(name, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("/var/lib/kea/dhcp4.leases", out);
}

void test_kea_subnet_pool_value_parsing(void) {
  struct cmc_ConfigField *dhcp4, *subnet4, *subnet_dict, *pools, *pool_dict,
      *pool_val;

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &dhcp4);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(dhcp4, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("subnet4", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &subnet4);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, subnet4);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &subnet_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnet4, subnet_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("pools", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &pools);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnet_dict, pools);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &pool_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(pools, pool_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("pool", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &pool_val);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(pool_dict, pool_val);
  TEST_ASSERT_NULL(err);

  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  char *out = NULL;
  err = cmc_field_get_str(pool_val, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("192.168.1.100 - 192.168.1.200", out);
}

void test_kea_option_data_second_value_parsing(void) {
  struct cmc_ConfigField *dhcp4, *subnet4, *subnet4_dict, *opt_data, *opt_dict,
      *name, *data;

  err = cmc_config_create(
      &(struct cmc_ConfigSettings){.supported_paths =
                                       (char *[]){(char *)KEA_CONFIG_PATH},
                                   .paths_length = 1,
                                   .name = "kea",
                                   .log_func = NULL},
      &config);
  TEST_ASSERT_NULL(err);

  // Build tree structure
  err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &dhcp4);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(dhcp4, config);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("subnet4", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &subnet4);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, subnet4);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &subnet4_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnet4, subnet4_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("option_data", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &opt_data);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnet4_dict, opt_data);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &opt_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(opt_data, opt_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(opt_dict, name);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("data", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &data);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(opt_dict, data);
  TEST_ASSERT_NULL(err);

  // Parse
  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // Assert structure for second option in array
  TEST_ASSERT_EQUAL_UINT32(2, opt_data->_self.subnodes_len);
  struct cmc_TreeNode *opt1_node = opt_data->_self.subnodes[1];
  TEST_ASSERT_NOT_NULL(opt1_node);

  struct cmc_ConfigField *opt1_dict = cmc_field_of_node(opt1_node);
  TEST_ASSERT_EQUAL_UINT32(2, opt1_dict->_self.subnodes_len);

  struct cmc_TreeNode *opt1_name_node = opt1_dict->_self.subnodes[0];
  struct cmc_TreeNode *opt1_data_node = opt1_dict->_self.subnodes[1];
  TEST_ASSERT_NOT_NULL(opt1_name_node);
  TEST_ASSERT_NOT_NULL(opt1_data_node);

  struct cmc_ConfigField *opt1_name = cmc_field_of_node(opt1_name_node);
  struct cmc_ConfigField *opt1_data = cmc_field_of_node(opt1_data_node);

  // Value assertions
  char *out = NULL;
  err = cmc_field_get_str(opt1_name, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("domain-name-servers", out);

  err = cmc_field_get_str(opt1_data, &out);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("8.8.8.8", out);
}

void test_parse_full_kea_env_file(void) {
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

  // Top-level dict: dhcp4
  struct cmc_ConfigField *dhcp4;
  err = cmc_field_create("dhcp4", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &dhcp4);
  TEST_ASSERT_NULL(err);
  err = cmc_config_add_field(dhcp4, config);
  TEST_ASSERT_NULL(err);

  // interfaces_config → interfaces[] → "eth0"
  struct cmc_ConfigField *ifcfg, *ifaces, *iface0;
  err = cmc_field_create("interfaces_config", cmc_ConfigFieldTypeEnum_DICT,
                         NULL, true, &ifcfg);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, ifcfg);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("interfaces", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &ifaces);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(ifcfg, ifaces);
  TEST_ASSERT_NULL(err);

  err =
      cmc_field_create("", cmc_ConfigFieldTypeEnum_STRING, NULL, true, &iface0);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(ifaces, iface0);
  TEST_ASSERT_NULL(err);

  // lease_database → type, persist, name
  struct cmc_ConfigField *lease, *type, *persist, *name;
  err = cmc_field_create("lease_database", cmc_ConfigFieldTypeEnum_DICT, NULL,
                         true, &lease);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, lease);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("type", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &type);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(lease, type);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("persist", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &persist);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(lease, persist);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(lease, name);
  TEST_ASSERT_NULL(err);

  // subnet4[] → {subnet, pools[], option_data[], valid_lifetime}
  struct cmc_ConfigField *subnets, *subnet, *subnet_val;
  err = cmc_field_create("subnet4", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &subnets);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, subnets);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &subnet);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnets, subnet);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("subnet", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &subnet_val);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnet, subnet_val);
  TEST_ASSERT_NULL(err);

  // pools
  struct cmc_ConfigField *pools, *pool_dict, *pool_val;
  err = cmc_field_create("pools", cmc_ConfigFieldTypeEnum_ARRAY, NULL, true,
                         &pools);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnet, pools);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true,
                         &pool_dict);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(pools, pool_dict);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("pool", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &pool_val);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(pool_dict, pool_val);
  TEST_ASSERT_NULL(err);

  // option_data → [dict, dict]
  struct cmc_ConfigField *opts, *opt0, *opt0_name, *opt0_data;
  err = cmc_field_create("option_data", cmc_ConfigFieldTypeEnum_ARRAY, NULL,
                         true, &opts);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(subnet, opts);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("", cmc_ConfigFieldTypeEnum_DICT, NULL, true, &opt0);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(opts, opt0);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("name", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &opt0_name);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(opt0, opt0_name);
  TEST_ASSERT_NULL(err);

  err = cmc_field_create("data", cmc_ConfigFieldTypeEnum_STRING, NULL, true,
                         &opt0_data);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(opt0, opt0_data);
  TEST_ASSERT_NULL(err);

  // valid_lifetime
  struct cmc_ConfigField *lifetime;
  err = cmc_field_create("valid_lifetime", cmc_ConfigFieldTypeEnum_INT, NULL,
                         true, &lifetime);
  TEST_ASSERT_NULL(err);
  err = cmc_field_add_subfield(dhcp4, lifetime);
  TEST_ASSERT_NULL(err);

  // ✅ Parse the config
  err = parser.parse(strlen(KEA_CONFIG_PATH), KEA_CONFIG_PATH, NULL, config);
  TEST_ASSERT_NULL(err);

  // ✅ Validate values
  char *s = NULL;
  int v = -1;

  err = cmc_field_get_str(iface0, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("eth0", s);

  err = cmc_field_get_str(type, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("memfile", s);

  err = cmc_field_get_str(persist, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("true", s);

  err = cmc_field_get_str(name, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("/var/lib/kea/dhcp4.leases", s);

  err = cmc_field_get_str(subnet_val, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("192.168.1.0/24", s);

  err = cmc_field_get_str(pool_val, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("192.168.1.100 - 192.168.1.200", s);

  err = cmc_field_get_str(opt0_name, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("routers", s);

  err = cmc_field_get_str(opt0_data, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("192.168.1.1", s);

  TEST_ASSERT_EQUAL_UINT32(2, opts->_self.subnodes_len);
  struct cmc_ConfigField *opt1_dict =
      cmc_field_of_node(opts->_self.subnodes[1]);
  struct cmc_ConfigField *opt1_name =
      cmc_field_of_node(opt1_dict->_self.subnodes[0]);
  struct cmc_ConfigField *opt1_data =
      cmc_field_of_node(opt1_dict->_self.subnodes[1]);

  err = cmc_field_get_str(opt1_name, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("domain-name-servers", s);

  err = cmc_field_get_str(opt1_data, &s);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_STRING("8.8.8.8", s);

  err = cmc_field_get_int(lifetime, &v);
  TEST_ASSERT_NULL(err);
  TEST_ASSERT_EQUAL_INT(3600, v);
}
