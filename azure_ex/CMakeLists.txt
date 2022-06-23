set(COMPONENT_ADD_INCLUDEDIRS "azure-sdk-for-c/sdk/inc")
set(az_sdk_dir azure-sdk-for-c/sdk/src/azure)
set(COMPONENT_SRCS  ${az_sdk_dir}/core/az_base64.c
                    ${az_sdk_dir}/core/az_context.c
                    ${az_sdk_dir}/core/az_http_pipeline.c
                    ${az_sdk_dir}/core/az_http_policy.c
                    ${az_sdk_dir}/core/az_http_policy_logging.c
                    ${az_sdk_dir}/core/az_http_policy_retry.c
                    ${az_sdk_dir}/core/az_http_request.c
                    ${az_sdk_dir}/core/az_http_response.c
                    ${az_sdk_dir}/core/az_json_reader.c
                    ${az_sdk_dir}/core/az_json_token.c
                    ${az_sdk_dir}/core/az_json_writer.c
                    ${az_sdk_dir}/core/az_log.c
                    ${az_sdk_dir}/core/az_precondition.c
                    ${az_sdk_dir}/core/az_span.c
                    ${az_sdk_dir}/iot/az_iot_hub_client.c
                    ${az_sdk_dir}/iot/az_iot_common.c
                    ${az_sdk_dir}/iot/az_iot_hub_client_sas.c
                    ${az_sdk_dir}/iot/az_iot_hub_client_telemetry.c
                    ${az_sdk_dir}/iot/az_iot_hub_client_c2d.c
                    ${az_sdk_dir}/iot/az_iot_hub_client_twin.c
                    ${az_sdk_dir}/iot/az_iot_hub_client_methods.c
                    ${az_sdk_dir}/iot/az_iot_hub_client_commands.c
                    ${az_sdk_dir}/iot/az_iot_hub_client_properties.c)

#set(COMPONENT_REQUIRES "mbedtls")
#set(COMPONENT_PRIV_REQUIRES "jsmn")

register_component()
