source ~/.bashrc
conda activate base

module rm compiler/rocm/2.9
module load compiler/rocm/4.0.1

export MIOPEN_DEBUG_DISABLE_FIND_DB=1
export MIOPEN_DEBUG_CONV_WINOGRAD=0
export MIOPEN_DEBUG_CONV_IMPLICIT_GEMM=0
export MIOPEN_FIND_MODE=5
export LD_LIBRARY_PATH=/public/home/acddp272rz/anaconda3/envs/pytorch-1.9/lib:$LD_LIBRARY_PATH

python train.py -s 1 -c 3 -b 16 -e 30 -l 0.0001
