#include <gtest/gtest.h>
#include <string>
#include "../disassembler.h"

using namespace testing::ext;
namespace panda::disasm{
class DisasmTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(DisasmTest, disasm_column_numbers, TestSize.Level1)
{
    std::cout << "执行到了获取abc文件之前了" << std::endl;
    const std::string file_name = GRAPH_TEST_ABC_DIR "disasm_column_numbers.abc";
    std::cout << "abc filename:" << file_name << std::endl;
    panda::disasm::Disassembler disasm {};
    std::cout << "执行到初始化之后了" << std::endl;
    disasm.Disassemble(file_name, false, false);
    std::cout << "执行到Dissaemble" << std::endl;
    disasm.CollectInfo();
    std::cout << "执行到CollectInfo" << std::endl;
    std::string number_str = "848484128128412161281281216128114158208158404086860";
    std::cout << "number_str: " << number_str << std::endl;
    std::string column_number_str = disasm.GetColumnNumber();
    std::cout << "column_number_str: " << column_number_str << std::endl;
    EXPECT_TRUE(number_str == column_number_str);
}
}
