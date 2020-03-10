function getRequiredPages(bytes) {
    const pageSize = 64 * 1024;   // 64 KB Page size per specification
    return Math.floor(bytes/ pageSize) + 1;
}