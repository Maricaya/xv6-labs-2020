# 调试办法
gdb server: make CPUS=1 qemu-gdb 
gdb client: gdb-multiarch 
// kernel/kernel

## 问题
- fs.img 被占用？
  - lsof fs.img 查看占用情况
  - kill pid
