// Verify options-widget virtual slots and seed indirect targets for read-only exports.
// @category HD2
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.util.LinkedHashSet;
import java.util.Set;

public class DefineOptionsWidgetFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x400000L)
            throw new IllegalArgumentException("Expected fingerprinted base client");
        long[][] slots = {
            {0x814210L, 0x2c, 0x664b90}, {0x814210L, 0x6c, 0x665010},
            {0x814210L, 0x30, 0x665430}, {0x814210L, 0x34, 0x6654b0},
            {0x814210L, 0x40, 0x665500}, {0x814210L, 0x48, 0x6656d0},
            {0x814210L, 0x50, 0x665720}, {0x814210L, 0x7c, 0x665760},
            {0x814210L, 0x4c, 0x6658c0}, {0x814210L, 0x1c, 0x6659c0},
            {0x815398L, 0x2c, 0x64f1c0}, {0x815398L, 0x6c, 0x64f550},
            {0x815398L, 0x40, 0x64f900}, {0x815398L, 0x30, 0x64f960},
            {0x815398L, 0x34, 0x64f9b0}, {0x815398L, 0x38, 0x64fa00},
            {0x815398L, 0x48, 0x64fa10}, {0x815398L, 0x7c, 0x64fa50},
            {0x815178L, 0x2c, 0x6541e0}, {0x815178L, 0x6c, 0x6546f0},
            {0x815178L, 0x30, 0x654840}, {0x815178L, 0x34, 0x6548b0},
            {0x815178L, 0x5c, 0x654910}, {0x815178L, 0x38, 0x654920},
            {0x815178L, 0x40, 0x654930}, {0x815178L, 0x7c, 0x6541a0}
        };
        Set<Long> entries = new LinkedHashSet<>();
        for (long[] slot : slots) {
            Address address = toAddr(slot[0] + slot[1]);
            if (Integer.toUnsignedLong(getInt(address)) != slot[2])
                throw new IllegalStateException("Unexpected options widget slot " + address);
            entries.add(slot[2]);
        }
        // Observer pointers passed by the original page builders.
        long[][] observers = {
            {0x61de6cL, 0x61fca0L}, {0x61ddf6L, 0x6202a0L},
            {0x61f2cfL, 0x620440L}, {0x61f48fL, 0x620720L}
        };
        for (long[] observer : observers) {
            Address site = toAddr(observer[0]);
            if (getByte(site) != 0x68 || Integer.toUnsignedLong(getInt(site.add(1))) != observer[1])
                throw new IllegalStateException("Unexpected options observer reference " + site);
            entries.add(observer[1]);
        }
        for (long entry : entries) {
            Address address = toAddr(entry);
            Function function = getFunctionAt(address);
            if (function == null) {
                Function containing = getFunctionContaining(address);
                if (containing != null)
                    throw new IllegalStateException(address + " overlaps " + containing.getEntryPoint());
                if (getInstructionAt(address) == null && !disassemble(address))
                    throw new IllegalStateException("Cannot disassemble " + address);
                function = createFunction(address, null);
                if (function == null) throw new IllegalStateException("Cannot define " + address);
            }
            println("Options widget target: " + function.getEntryPoint());
        }
    }
}
