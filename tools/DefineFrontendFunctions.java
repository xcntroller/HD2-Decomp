// Seed verified indirect front-end targets; use -readOnly for transient research exports.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.util.LinkedHashSet;
import java.util.Set;

public class DefineFrontendFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x400000L) {
            throw new IllegalArgumentException("Expected the fingerprinted base client");
        }
        long[][] tables = {
            {0x815e30L, 0x6c1660L, 0x66ad10L, 0x66b140L, 0x66b3e0L,
             0x66be90L, 0x66bea0L, 0x66de00L, 0x6c1650L,
             0x67ea00L, 0x67ea10L, 0x67ea20L, 0x67ea30L, 0x67ea40L,
             0x626b20L, 0x627060L, 0x626950L, 0x626340L},
            {0x815bc8L, 0x6c0dc0L, 0x688040L, 0x66ed30L, 0x66ed50L,
             0x67ec60L, 0x66ed80L, 0x66eda0L, 0x6c0db0L}
        };
        Set<Long> entries = new LinkedHashSet<>();
        for (long[] table : tables) {
            for (int index = 1; index < table.length; ++index) {
                Address slot = toAddr(table[0] + (index - 1) * 4);
                if (Integer.toUnsignedLong(getInt(slot)) != table[index]) {
                    throw new IllegalStateException("Unexpected front-end vtable slot " + slot);
                }
                entries.add(table[index]);
            }
        }
        if (getByte(toAddr(0x66ad66L)) != 0x68
                || Integer.toUnsignedLong(getInt(toAddr(0x66ad67L))) != 0x66e0f0L) {
            throw new IllegalStateException("Unexpected menu scene callback reference");
        }
        entries.add(0x66e0f0L);
        for (long entry : entries) {
            Address address = toAddr(entry);
            Function function = getFunctionAt(address);
            if (function == null) {
                Function containing = getFunctionContaining(address);
                if (containing != null) {
                    throw new IllegalStateException(address + " overlaps " + containing.getEntryPoint());
                }
                if (getInstructionAt(address) == null && !disassemble(address)) {
                    throw new IllegalStateException("Cannot disassemble front-end target " + address);
                }
                function = createFunction(address, null);
                if (function == null) {
                    throw new IllegalStateException("Cannot define front-end target " + address);
                }
            }
            println("Front-end target: " + function.getEntryPoint());
        }
    }
}
