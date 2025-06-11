/*****************************************************************************************
 *              MIT License *
 *                                                                                       *
 * Copyright (c) 2022 G. Cherchi, F. Pellacini, M. Attene and M. Livesu *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this  * software and associated documentation files (the "Software"), to
 * deal in the Software * without restriction, including without limitation the
 * rights to use, copy, modify,    * merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to    * permit persons to whom the
 * Software is furnished to do so, subject to the following   * conditions: *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in
 * all copies * or substantial portions of the Software. *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED,   * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A         * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT    * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION     * OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        * SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. *
 *                                                                                       *
 * Authors: * Gianmarco Cherchi (g.cherchi@unica.it) *
 *      https://www.gianmarcocherchi.com *
 *                                                                                       *
 *      Fabio Pellacini (fabio.pellacini@uniroma1.it) *
 *      https://pellacini.di.uniroma1.it *
 *                                                                                       *
 *      Marco Attene (marco.attene@ge.imati.cnr.it) *
 *      https://www.cnr.it/en/people/marco.attene/ *
 *                                                                                       *
 *      Marco Livesu (marco.livesu@ge.imati.cnr.it) *
 *      http://pers.ge.imati.cnr.it/livesu/ *
 *                                                                                       *
 * ***************************************************************************************/

#ifdef _MSC_VER // Workaround for known bugs and issues on MSVC
#define _HAS_STD_BYTE                                                          \
  0 // https://developercommunity.visualstudio.com/t/error-c2872-byte-ambiguous-symbol/93889
#define NOMINMAX // https://stackoverflow.com/questions/1825904/error-c2589-on-stdnumeric-limitsdoublemin
#endif

#include "booleans.h"

std::vector<std::string> files;

int main(int argc, char **argv) {
  BoolOp op;
  std::string file_out;

  if (argc < 5) {
    std::cout << "syntax error!" << std::endl;
    std::cout << "./exact_boolean BOOL_OPERATION (intersection OR union OR "
                 "subtraction) input1.obj input2.obj output.obj"
              << std::endl;
    return -1;
  } else {
    if (strcmp(argv[1], "intersection") == 0)
      op = INTERSECTION;
    else if (strcmp(argv[1], "union") == 0)
      op = UNION;
    else if (strcmp(argv[1], "subtraction") == 0)
      op = SUBTRACTION;
    else if (strcmp(argv[1], "xor") == 0)
      op = XOR;
  }

  for (int i = 2; i < (argc - 1); i++)
    files.emplace_back(argv[i]);

  file_out = argv[argc - 1];

  std::vector<double> in_coords, bool_coords;
  std::vector<uint> in_tris, bool_tris;
  std::vector<uint> in_labels;
  std::vector<std::bitset<NBIT>> bool_labels;

  // loadMultipleFiles(files, in_coords, in_tris, in_labels);
  // generate a easy test case, get the coordinate from a tetrahedron to label 0
  // generate a triangle to be lable 1
  // 生成四面体的顶点坐标
  in_coords = {
      0.0, 0.0, 0.0, // 顶点0
      1.0, 0.0, 0.0, // 顶点1
      0.0, 1.0, 0.0, // 顶点2
      0.0, 0.0, 1.0  // 顶点3
  };

  // 生成四面体的四个三角形面
  in_tris = {
      0, 1, 2, // 底面
      0, 1, 3, // 侧面1
      1, 2, 3, // 侧面2
      2, 0, 3  // 侧面3
  };

  // 添加一个与四面体相交的三角形
  // 在四面体中间位置添加一个水平三角形
  in_coords.insert(in_coords.end(), {
                                        0.5, -0.2, 0.5, // 顶点4
                                        0.5, 0.8, 0.5,  // 顶点5
                                        -0.2, 0.3, 0.5  // 顶点6
                                    });

  in_tris.insert(in_tris.end(), {
                                    4, 5, 6 // 相交的三角形
                                });

  // 设置标签：四面体的面标签为0，相交三角形的标签为1
  in_labels = {0, 0, 0, 0,
               1}; // 前4个面属于四面体(标签0)，最后一个面是相交三角形(标签1)
  // 初始化必要的数据结构
  point_arena arena;
  std::vector<genericPoint *> arr_verts;
  std::vector<uint> arr_in_tris, arr_out_tris;
  std::vector<std::bitset<NBIT>> arr_in_labels;
  std::vector<DuplTriInfo> dupl_triangles;
  Labels labels;
  cinolib::Octree octree;

  // 执行arrangement
  customArrangementPipeline(in_coords, in_tris, in_labels, arr_in_tris,
                            arr_in_labels, arena, arr_verts, arr_out_tris,
                            labels, octree, dupl_triangles, true);

  // 创建FastTrimesh
  FastTrimesh tm(arr_verts, arr_out_tris, true);

  // 只保留label为1的三角形
  tm.resetTrianglesInfo();
  uint num_tris = 0;
  for (uint t_id = 0; t_id < tm.numTris(); t_id++) {
    if (labels.surface[t_id][1]) { // 检查label 1
      tm.setTriInfo(t_id, 1);
      num_tris++;
    }
  }

  // 准备输出数据
  std::vector<double> out_coords;
  std::vector<uint> out_tris;
  std::vector<std::bitset<NBIT>> out_labels;

  // 生成最终结果
  computeFinalExplicitResult(tm, labels, num_tris, out_coords, out_tris,
                             out_labels, true);

  // 写入OBJ文件
  cinolib::write_OBJ(file_out.c_str(), out_coords, out_tris, {});
  //   booleanPipeline(in_coords, in_tris, in_labels, op, bool_coords,
  //   bool_tris,
  //                   bool_labels);

  //   cinolib::write_OBJ(file_out.c_str(), bool_coords, bool_tris, {});

  return 0;
}