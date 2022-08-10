# Find OpenSSL

find_package(OpenSSL 1.1 REQUIRED)
 
target_link_libraries(${PROJECT_NAME}
    OpenSSL::Crypto
)
